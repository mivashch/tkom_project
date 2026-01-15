#pragma once
#include "ast.h"
#include <ostream>

namespace minilang::ast {

    struct AstPrinter : ASTVisitor {
        std::ostream& os;
        int indent = 0;

        explicit AstPrinter(std::ostream& out) : os(out) {}

        void dump(Node& node) {
            node.accept(*this);
        }

    private:
        void pad() {
            for (int i = 0; i < indent; i++) os << "  ";
        }

    public:
        void visit(LiteralExpr& e) override;
        void visit(IdentifierExpr& e) override;
        void visit(UnaryExpr& e) override;
        void visit(BinaryExpr& e) override;
        void visit(CallExpr& e) override;
        void visit(TupleExpr& e) override;
        void visit(ExprStmt& s) override;
        void visit(VarDeclStmt& s) override;
        void visit(AssignExpr& s) override;
        void visit(ReturnStmt& s) override;
        void visit(BlockStmt& s) override;
        void visit(IfStmt& s) override;
        void visit(ForStmt& s) override;
        void visit(FuncDeclStmt& s) override;
        void visit(Program& p) override;
    };

}
