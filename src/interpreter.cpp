#include "interpreter.h"
#include <iostream>
#include <limits>

using namespace minilang;
using namespace minilang::ast;

template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;


Value& Environment::lookup(const std::string& name) {
    if (vars.count(name)) return vars[name];
    if (parent) return parent->lookup(name);
    throw RuntimeError({}, "Undefined variable: " + name);
}
bool Environment::existsLocal(const std::string& name) const {
    return vars.count(name);
}

bool Environment::exists(const std::string& name) const {
    if (vars.count(name)) return true;
    if (parent) return parent->exists(name);
    return false;
}

void Environment::define(const std::string& name, Value v) {
    vars[name] = std::move(v);
}


Interpreter::Interpreter() {
    env_ = new Environment;

    env_->vars["print"] = std::make_shared<Function>(Function{
        {"x"},
        nullptr,
        [](const std::vector<Value>& args) -> Value {
            std::visit(OutputValue{std::cout}, args[0]);
            std::cout << "\n";
            return {};
        }
    });
}


void Interpreter::execute(Program& p) {
    p.accept(*this);
}

bool Interpreter::isTruthy(const Value& v) {
    return std::visit([](auto&& x) -> bool {
        using T = std::decay_t<decltype(x)>;

        if constexpr (std::is_same_v<T, std::monostate>)
            throw std::runtime_error("Invalid condition value");

        else if constexpr (std::is_same_v<T, bool>)
            return x;

        else if constexpr (std::is_same_v<T, long long>)
            return x != 0;

        else if constexpr (std::is_same_v<T, double>)
            return x != 0.0;

        else if constexpr (std::is_same_v<T, std::string>)
            return !x.empty();
        return false ;
    }, v);
}

long long Interpreter::asInt(const Value& v) {
    if (auto p = std::get_if<long long>(&v)) return *p;
    throw RuntimeError({}, "Expected integer");
}

double Interpreter::asDouble(const Value& v) {
    if (auto p = std::get_if<double>(&v)) return *p;
    if (auto i = std::get_if<long long>(&v)) return *i;
    throw RuntimeError({}, "Expected number");
}

bool Interpreter::asBool(const Value& v) {
    return std::visit([](auto&& x) -> bool {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, std::monostate>)
            return false;
        else if constexpr (std::is_same_v<T, bool>)
            return x;
        else if constexpr (std::is_same_v<T, long long>)
            return x != 0;
        else if constexpr (std::is_same_v<T, double>)
            return x != 0.0;
        else if constexpr (std::is_same_v<T, std::string>)
            // return !x.empty();
            throw RuntimeError({}, "Invalid boolean context");

        else
            throw RuntimeError({}, "Invalid boolean context");
    }, v);
}

Value &Interpreter::getLastValue() {
    return lastValue_;
}

long long Interpreter::toInt(const Value& v) {
    return std::visit(overloaded {
        [](long long x) { return x; },
        [](double x)    { return static_cast<long long>(x); },
        [](bool x)      { return x ? 1LL : 0LL; },
        [](const std::string& s) {
            try {
                return std::stoll(s);
            } catch (...) {
                throw RuntimeError({}, "Cannot convert string to int: " + s);
            }
        },
        [](auto&) -> long long {
            throw RuntimeError({}, "Invalid int conversion");
        }
    }, v);
}

double Interpreter::toNumber(const Value& v) {
    return std::visit(overloaded {
        [](long long x) { return static_cast<double>(x); },
        [](double x)    { return x; },
        [](bool x)      { return x ? 1.0 : 0.0; },
        [](const std::string& s) {
            try {
                return std::stod(s);
            } catch (...) {
                throw RuntimeError({}, "Cannot convert string to number: " + s);
            }
        },
        [](auto&) -> double {
            throw RuntimeError({}, "Invalid numeric conversion");
        }
    }, v);
}

std::string Interpreter::toString(const Value& v) {
    return std::visit(overloaded{
        [](const std::string& s) -> std::string {return s;},
        [](long long x) -> std::string {return std::to_string(x);},
        [](double x) -> std::string {return std::to_string(x);},
        [](bool x) -> std::string {return x ? std::string("true") : std::string("false");},
        [](std::monostate) -> std::string {return std::string("null");},
        [](auto&) -> std::string {throw RuntimeError({}, "Cannot convert to string");}
    }, v);
}


bool Interpreter::toBool(const Value& v) {
    return std::visit(overloaded {
        [](bool b)        { return b; },
        [](long long x)   { return x != 0; },
        [](double x)      { return x != 0.0; },
        [](const std::string& s) { return !s.empty(); },
        [](std::monostate){ return false; },
        [](auto&) -> bool {
            throw RuntimeError({}, "Invalid boolean context");
        }
    }, v);
}


void Interpreter::visit(LiteralExpr& e) {
    lastValue_ = std::visit(
        [](auto&& v) -> Value {
            return v;
        },
        e.value
    );
}


void Interpreter::visit(IdentifierExpr& e) {
    lastValue_ = env_->lookup(e.name);
}

void Interpreter::visit(AssignExpr& e) {
    e.value->accept(*this);

    if (env_->exists(e.target)) {
        env_->lookup(e.target) = lastValue_;
    } else {
        env_->define(e.target, lastValue_);
    }
}


void Interpreter::visit(UnaryExpr& e) {
    e.rhs->accept(*this);
    if (e.op == "-") {
        lastValue_ = -std::get<long long>(lastValue_);
    }
}

void Interpreter::visit(BinaryExpr& e) {
    e.lhs->accept(*this);
    Value l = lastValue_;

    e.rhs->accept(*this);
    Value r = lastValue_;

    const std::string& op = e.op;

    if (e.op == "+") {
        if (std::holds_alternative<std::string>(l)) {
            lastValue_ = toString(l) + toString(r);
        }
        else {
            lastValue_ = toNumber(l) + toNumber(r);
        }
        return;
    }

    if (op == "-") {
        lastValue_ = toNumber(l) - toNumber(r);
        return;
    }

    if (op == "*") {
        lastValue_ = toNumber(l) * toNumber(r);
        return;
    }

    if (op == "/") {
        double denom = toNumber(r);
        if (denom == 0.0)
            lastValue_ = std::numeric_limits<double>::infinity();
        else
            lastValue_ = toNumber(l) / denom;
        return;
    }

    if (op == "%") {
        long long a = toInt(l);
        long long b = toInt(r);
        if (b == 0)
            throw RuntimeError(e.pos, "Modulo by zero");
        lastValue_ = a % b;
        return;
    }

    if (op == "==" || op == "!=" ||
        op == "<"  || op == "<=" ||
        op == ">"  || op == ">=") {

        bool result;

            double a = toNumber(l);
            double b = toNumber(r);

            if (op == "==") result = a == b;
            else if (op == "!=") result = a != b;
            else if (op == "<") result = a < b;
            else if (op == "<=") result = a <= b;
            else if (op == ">") result = a > b;
            else result = a >= b;


        lastValue_ = result;
        return;
    }

    if (op == "&&") {
        lastValue_ = asBool(l) && asBool(r);
        return;
    }

    if (op == "||") {
        lastValue_ = asBool(l) || asBool(r);
        return;
    }

     if (e.op == "&*&") {
        auto baseFn = std::get_if<FunctionPtr>(&l);
        auto decoFn = std::get_if<FunctionPtr>(&r);

        if (!baseFn || !*baseFn || !decoFn || !*decoFn)
            throw RuntimeError(e.pos, "Decorator requires two functions");

        Function& base = **baseFn;
        Function& deco = **decoFn;

        if (deco.params.size() != base.params.size() + 1)
            throw RuntimeError(
                e.pos,
                "Decorator must take (function + base arguments)"
            );

        auto result = std::make_shared<Function>();

        result->params = base.params;

        result->builtin =
            [this, base = *baseFn, deco = *decoFn]
            (const std::vector<Value>& args) -> Value {

                std::vector<Value> decoArgs;
                decoArgs.reserve(args.size() + 1);

                decoArgs.push_back(Value(base));
                decoArgs.insert(decoArgs.end(), args.begin(), args.end());

                return invoke(Value(deco), decoArgs, {});
        };

        lastValue_ = result;
    }

    else if (e.op == "=>>") {
        std::vector<Value> boundArgs;

        if (auto tuplePtr = std::get_if<std::shared_ptr<TupleValue>>(&l)) {
            boundArgs = (*tuplePtr)->elements;
        } else {
            boundArgs.push_back(l);
        }

        auto fnPtr = std::get_if<FunctionPtr>(&r);
        if (!fnPtr || !*fnPtr)
            throw RuntimeError(e.pos, "Right side of =>> must be function");

        auto& fn = **fnPtr;

        if (boundArgs.size() > fn.params.size())
            throw RuntimeError(e.pos, "Too many bound arguments");

        auto result = std::make_shared<Function>();

        result->params.assign(
            fn.params.begin() + boundArgs.size(),
            fn.params.end()
        );

        result->builtin =
            [this, fnPtr = *fnPtr, boundArgs](const std::vector<Value>& callArgs) -> Value {

                std::vector<Value> fullArgs;
                fullArgs.reserve(boundArgs.size() + callArgs.size());

                fullArgs.insert(fullArgs.end(), boundArgs.begin(), boundArgs.end());
                fullArgs.insert(fullArgs.end(), callArgs.begin(), callArgs.end());

                return invoke(
                    Value(fnPtr),
                    fullArgs,
                    {}
                );
        };

        lastValue_ = result;
    }
    else
        throw RuntimeError(e.pos, "Unknown binary operator: " + e.op);
}

Value Interpreter::invoke(
    const Value& callee,
    const std::vector<Value>& args,
    Position pos
) {
    auto fnPtr = std::get_if<FunctionPtr>(&callee);
    if (!fnPtr || !*fnPtr)
        throw RuntimeError(pos, "Value is not callable");

    Function& fn = **fnPtr;

    if (args.size() != fn.params.size())
        throw RuntimeError(
            pos,
            "Wrong number of arguments: expected " +
            std::to_string(fn.params.size()) +
            ", got " +
            std::to_string(args.size())
        );

    if (fn.builtin)
        return fn.builtin(args);

    Environment local;
    local.parent = env_;

    for (size_t i = 0; i < args.size(); ++i)
        local.vars[fn.params[i]] = args[i];

    Environment* saved = env_;
    env_ = &local;

    try {
        fn.body->accept(*this);
        env_ = saved;
        return std::monostate{};
    }
    catch (ReturnSignal& r) {
        env_ = saved;
        return r.value;
    }
}

void Interpreter::visit(CallExpr& e) {
    e.callee->accept(*this);
    Value callee = lastValue_;

    std::vector<Value> args;
    for (auto& a : e.args) {
        a->accept(*this);
        args.push_back(lastValue_);
    }

    lastValue_ = invoke(callee, args, e.pos);
}

void Interpreter::visit(ExprStmt& s) {
    if (s.expr) s.expr->accept(*this);
}

void Interpreter::visit(VarDeclStmt& s) {
    s.init->accept(*this);

    if (env_->existsLocal(s.name)) {
        throw RuntimeError(s.pos, "Variable redeclared: " + s.name);
    }

    env_->define(s.name, lastValue_);
}


void Interpreter::visit(ReturnStmt& s) {
    if (s.value)
        s.value->accept(*this);
    else
        lastValue_ = std::monostate{};

    throw ReturnSignal{ lastValue_ };
}


void Interpreter::visit(BlockStmt& b) {
    Environment local;
    local.parent = env_;
    Environment* saved = env_;
    env_ = &local;

    for (auto& st : b.stmts)
        st->accept(*this);

    env_ = saved;
}

void Interpreter::visit(IfStmt& s) {
    s.cond->accept(*this);

    if (isTruthy(lastValue_)) {
        s.thenBlock->accept(*this);
    } else if (s.elseBlock) {
        s.elseBlock->accept(*this);
    }
}

bool Interpreter::forConditionHolds(const ForStmt& s) {
    if (!s.cond)
        return true; //if we dont want infinit loop for(;;) just change to false

    s.cond->accept(*this);
    return asBool(lastValue_);
}

void Interpreter::visit(ForStmt& s) {
    if (s.initExpr)
        s.initExpr->accept(*this);

    while (forConditionHolds(s)) {
        s.body->accept(*this);

        if (s.post)
            s.post->accept(*this);
    }
}


void Interpreter::visit(FuncDeclStmt& s) {
    auto fn = std::make_shared<Function>();
    fn->params.reserve(s.params.size());

    for (auto& [name, _] : s.params)
        fn->params.push_back(name);

    fn->body = std::move(s.body);
    fn->builtin = nullptr;

    env_->define(s.name, fn);
}


void Interpreter::visit(Program& p) {
    for (auto& s : p.stmts)
        s->accept(*this);
}

void Interpreter::visit(TupleExpr& e) {
    auto tuple = std::make_shared<TupleValue>();

    for (auto& el : e.elements) {
        el->accept(*this);
        tuple->elements.push_back(lastValue_);
    }

    lastValue_ = tuple;
}


