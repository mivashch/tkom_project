#pragma once
#include "ast.h"
#include "runtime_error.h"
#include <unordered_map>
#include <functional>
#include <ostream>
#include <ios>
#include <variant>

namespace minilang {
    struct Function;

    using FunctionPtr = std::shared_ptr<Function>;

    using Value = std::variant<
        std::monostate,
        long long,
        double,
        std::string,
        bool,
        FunctionPtr
    >;


    struct Function {
        std::vector<std::string> params;
        std::unique_ptr<ast::BlockStmt> body;
        std::function<Value(const std::vector<Value> &)> builtin;
    };

    struct Environment {
        std::unordered_map<std::string, Value> vars;
        Environment *parent = nullptr;

        Value &lookup(const std::string &name);

        bool existsLocal(const std::string &name) const;

        bool exists(const std::string &name) const;

        void define(const std::string &name, Value v);
    };

    struct OutputValue {
        std::ostream &os;

        void operator()(std::monostate) const {
        }

        void operator()(long long v) const {
            os << v;
        }

        void operator()(double v) const {
            os << v;
        }

        void operator()(const std::string &v) const {
            os << v;
        }

        void operator()(bool v) const {
            os << std::boolalpha << v;
        }

        void operator()(const std::shared_ptr<Function> &) const {
            os << "<function>";
        }
    };

    struct ReturnSignal {
        Value value;
    };


    class Interpreter : public ast::ASTVisitor {
    public:
        Interpreter();

        void execute(ast::Program &p);

        void visit(ast::LiteralExpr &) override;

        void visit(ast::IdentifierExpr &) override;

        void visit(ast::UnaryExpr &) override;

        void visit(ast::BinaryExpr &) override;

        void visit(ast::CallExpr &) override;

        void visit(ast::AssignExpr &) override;

        void visit(ast::ExprStmt &) override;

        void visit(ast::VarDeclStmt &) override;

        void visit(ast::ReturnStmt &) override;

        void visit(ast::BlockStmt &) override;

        void visit(ast::IfStmt &) override;

        void visit(ast::ForStmt &) override;

        void visit(ast::FuncDeclStmt &) override;

        void visit(ast::Program &) override;

        bool isTruthy(const Value &v);

        static long long asInt(const Value &v);

        static double asDouble(const Value &v);

        static bool asBool(const Value &v);
        bool forConditionHolds(const ast::ForStmt& s);


        Value &getLastValue();

    private:
        Environment *env_;
        std::unordered_map<std::string, FunctionPtr> functions_;

        Value lastValue_;
    };
}
