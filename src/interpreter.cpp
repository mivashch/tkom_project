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


VarSlot& Environment::lookupSlot(const std::string& name) {
    if (vars.count(name))
        return vars.at(name);
    if (parent)
        return parent->lookupSlot(name);
    throw RuntimeError({}, "Undefined variable: " + name);
}

Value& Environment::lookup(const std::string& name) {
    return lookupSlot(name).value;
}

void Environment::define(const std::string& name, Value v, bool isConst) {
    vars[name] = VarSlot{ std::move(v), isConst };
}

bool Environment::existsLocal(const std::string& name) const {
    return vars.count(name);
}

bool Environment::exists(const std::string& name) const {
    if (vars.count(name)) return true;
    if (parent) return parent->exists(name);
    return false;
}



Interpreter::Interpreter()  {
    root_ = std::make_unique<Environment>();
    env_ = root_.get();
    env_->define(
        "print",
        Value(std::make_shared<Function>(Function{
            {"x"},
            nullptr,
            [](const std::vector<Value>& args) -> Value {
                std::visit(OutputValue{std::cout}, args[0]);
                std::cout << "\n";
                return std::monostate{};
            }
        })),
       true
    );

}


void Interpreter::execute(Program& p) {
    p.accept(*this);
}

bool Interpreter::isTruthy(const Value& v) {
    return std::visit(overloaded{
        [](bool b) {
            return b;
        },
        [](long long x) {
            return x != 0;
        },
        [](double x) {
            return x != 0.0;
        },
        [](const std::string& s) {
            return !s.empty();
        },
        [](std::monostate) -> bool {
            throw RuntimeError({}, "Invalid condition value");
        },
        [](auto&) -> bool {
            throw RuntimeError({}, "Invalid condition value");
        }
    }, v);
}


long long Interpreter::asInt(const Value& v) {
    return std::visit(overloaded{
        [](long long x) -> long long {
            return x;
        },
        [](auto&) -> long long {
            throw RuntimeError({}, "Expected integer");
        }
    }, v);
}


double Interpreter::asDouble(const Value& v) {
    return std::visit(overloaded{
        [](double x) -> double {
            return x;
        },
        [](long long x) -> double {
            return static_cast<double>(x);
        },
        [](auto&) -> double {
            throw RuntimeError({}, "Expected number");
        }
    }, v);
}


bool Interpreter::asBool(const Value& v) {
    return std::visit(overloaded{
        [](bool b) -> bool {
            return b;
        },
        [](long long x) -> bool {
            return x != 0;
        },
        [](double x) -> bool {
            return x != 0.0;
        },
        [](std::monostate) -> bool {
            return false;
        },
        [](auto&) -> bool {
            throw RuntimeError({}, "Invalid boolean context");
        }
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
    Value rhs = lastValue_;

    if (env_->exists(e.target)) {
        auto& slot = env_->lookupSlot(e.target);

        if (slot.isConst)
            throw RuntimeError(
                e.pos,
                "Cannot assign to const variable '" + e.target + "'"
            );

        slot.value = rhs;
    }
    else {
        env_->define(
            e.target,
            rhs,
            false
        );
    }

    lastValue_ = rhs;
}




void Interpreter::visit(UnaryExpr& e) {
    e.rhs->accept(*this);

    if (e.op == "-") {
        lastValue_ = std::visit(overloaded{
            [](long long x) -> Value { return -x; },
            [](double x)    -> Value { return -x; },
            [](auto&) -> Value {
                throw RuntimeError({}, "Unary '-' expects numeric value");
            }
        }, lastValue_);
    }
}

Value Interpreter::evalAdd(const Value& l, const Value& r) {
    if (std::holds_alternative<std::string>(l))
        return toString(l) + toString(r);

    return toNumber(l) + toNumber(r);
}

Value Interpreter::evalDiv(const Value& l, const Value& r, Position pos) {
    double denom = toNumber(r);
    if (denom == 0.0)
        return std::numeric_limits<double>::infinity();
    return toNumber(l) / denom;
}

Value Interpreter::evalCompare(
    const BinaryOp& op,
    const Value& l,
    const Value& r
) {
    double a = toNumber(l);
    double b = toNumber(r);

    if (op == BinaryOp::Eq) return a == b;
    if (op == BinaryOp::Ne) return a != b;
    if (op == BinaryOp::Ls)  return a <  b;
    if (op == BinaryOp::Le) return a <= b;
    if (op == BinaryOp::Ge)  return a >  b;
    return a >= b;
}

Value Interpreter::evalSub(const Value& l, const Value& r) {
    return toNumber(l) - toNumber(r);
}

Value Interpreter::evalMul(const Value& l, const Value& r) {
    return toNumber(l) * toNumber(r);
}

Value Interpreter::evalMod(const Value& l, const Value& r, Position pos) {
    long long a = toInt(l);
    long long b = toInt(r);

    if (b == 0)
        throw RuntimeError(pos, "Modulo by zero");

    return a % b;
}

Value Interpreter::evalAnd(const Value& l, const Value& r) {
    return toBool(l) && toBool(r);
}

Value Interpreter::evalOr(const Value& l, const Value& r) {
    return toBool(l) || toBool(r);
}

Value Interpreter::evalDecorator(
    const Value& l,
    const Value& r,
    Position pos
) {
    auto baseFn = std::get_if<FunctionPtr>(&l);
    auto decoFn = std::get_if<FunctionPtr>(&r);

    if (!baseFn || !*baseFn || !decoFn || !*decoFn)
        throw RuntimeError(pos, "Decorator requires two functions");

    Function& base = **baseFn;
    Function& deco = **decoFn;

    if (deco.params.size() != base.params.size() + 1)
        throw RuntimeError(
            pos,
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

            return invoke(
                Value(deco),
                decoArgs,
                {}
            );
    };

    return result;
}

Value Interpreter::evalBind(
    const Value& l,
    const Value& r,
    Position pos
) {
    std::vector<Value> boundArgs;

    if (auto tuplePtr = std::get_if<std::shared_ptr<TupleValue>>(&l)) {
        boundArgs = (*tuplePtr)->elements;
    } else {
        boundArgs.push_back(l);
    }

    auto fnPtr = std::get_if<FunctionPtr>(&r);
    if (!fnPtr || !*fnPtr)
        throw RuntimeError(pos, "Right side of =>> must be function");

    Function& fn = **fnPtr;

    if (boundArgs.size() > fn.params.size())
        throw RuntimeError(pos, "Too many bound arguments");

    auto result = std::make_shared<Function>();

    result->params.assign(
        fn.params.begin() + boundArgs.size(),
        fn.params.end()
    );

    result->builtin =
        [this, fnPtr = *fnPtr, boundArgs]
        (const std::vector<Value>& callArgs) -> Value {

            std::vector<Value> fullArgs;
            fullArgs.reserve(boundArgs.size() + callArgs.size());

            fullArgs.insert(fullArgs.end(),
                            boundArgs.begin(),
                            boundArgs.end());

            fullArgs.insert(fullArgs.end(),
                            callArgs.begin(),
                            callArgs.end());

            return invoke(
                Value(fnPtr),
                fullArgs,
                {}
            );
    };

    return result;
}

bool Interpreter::isComparison(const BinaryOp& op) {
    return op == BinaryOp::Eq || op == BinaryOp::Ne||
           op == BinaryOp::Ls || op == BinaryOp::Le ||
           op == BinaryOp::Gt  || op == BinaryOp::Ge;
}

Value Interpreter::evalBinary(
    const BinaryOp& op,
    const Value& l,
    const Value& r,
    Position pos,
    std::string& ops
) {
    if (op == BinaryOp::Add)   return evalAdd(l, r);
    if (op == BinaryOp::Sub)   return evalSub(l, r);
    if (op == BinaryOp::Mul)   return evalMul(l, r);
    if (op == BinaryOp::Div)   return evalDiv(l, r, pos);
    if (op == BinaryOp::Mod)   return evalMod(l, r, pos);

    if (isComparison(op))
        return evalCompare(op, l, r);

    if (op == BinaryOp::And)  return evalAnd(l, r);
    if (op == BinaryOp::Or)  return evalOr(l, r);

    if (op == BinaryOp::Decorator) return evalDecorator(l, r, pos);
    if (op == BinaryOp::Bind) return evalBind(l, r, pos);

    throw RuntimeError(pos, "Unknown binary operator: " + ops);
}


void Interpreter::visit(BinaryExpr& e) {
    e.lhs->accept(*this);
    Value l = lastValue_;

    e.rhs->accept(*this);
    Value r = lastValue_;

    lastValue_ = evalBinary(e.opt, l, r, e.pos, e.op);
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
        throw RuntimeError(pos, "...");

    if (fn.builtin)
        return fn.builtin(args);

    Environment local;
    local.parent = env_;
    for (size_t i = 0; i < args.size(); ++i)
        local.define(fn.params[i], args[i], false);

    Environment* saved = env_;
    env_ = &local;

    exec_ = {};
    fn.body->accept(*this);

    env_ = saved;

    if (exec_.hasReturn) {
        Value val = exec_.value;
        exec_ = {};
        return val ;
    }

    return std::monostate{};
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

    if (env_->existsLocal(s.name))
        throw RuntimeError(s.pos, "Variable redeclared: " + s.name);

    env_->define(s.name, lastValue_, s.isConst);
}



void Interpreter::visit(ReturnStmt& s) {
    if (s.value)
        s.value->accept(*this);
    else
        lastValue_ = std::monostate{};

    exec_.hasReturn = true;
    exec_.value = lastValue_;
}



void Interpreter::visit(BlockStmt& b) {
    Environment local;
    local.parent = env_;
    Environment* saved = env_;
    env_ = &local;

    for (auto& st : b.stmts) {
        st->accept(*this);
        if (exec_.hasReturn)
            break;
    }
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

        if (exec_.hasReturn)
            break;

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

    env_->define(s.name, fn, true);
}


void Interpreter::visit(Program& p) {
    for (auto& s : p.stmts) {
        s->accept(*this);
        if (exec_.hasReturn)
            throw RuntimeError({}, "Return outside function");
    }
}


void Interpreter::visit(TupleExpr& e) {
    auto tuple = std::make_shared<TupleValue>();

    for (auto& el : e.elements) {
        el->accept(*this);
        tuple->elements.push_back(lastValue_);
    }

    lastValue_ = tuple;
}


