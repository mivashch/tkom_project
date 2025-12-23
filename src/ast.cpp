#include "ast.h"

namespace minilang::ast {
    Program::Program(std::vector<std::unique_ptr<Stmt> > statements) : Node() {
        for (auto it = statements.begin(); it != statements.end(); ++it) {
            stmts.push_back(std::move(*it));
        }
    }


    void LiteralExpr::accept(ASTVisitor& v) { v.visit(*this); }
    void IdentifierExpr::accept(ASTVisitor& v) { v.visit(*this); }
    void UnaryExpr::accept(ASTVisitor& v) { v.visit(*this); }
    void BinaryExpr::accept(ASTVisitor& v) { v.visit(*this); }
    void CallExpr::accept(ASTVisitor& v) { v.visit(*this); }
    void LambdaExpr::accept(ASTVisitor& v) { v.visit(*this); }
    void AssignExpr::accept(ASTVisitor& v) { v.visit(*this); }

    void ExprStmt::accept(ASTVisitor& v) { v.visit(*this); }
    void VarDeclStmt::accept(ASTVisitor& v) { v.visit(*this); }
    void ReturnStmt::accept(ASTVisitor& v) { v.visit(*this); }
    void BlockStmt::accept(ASTVisitor& v) { v.visit(*this); }
    void IfStmt::accept(ASTVisitor& v) { v.visit(*this); }
    void ForStmt::accept(ASTVisitor& v) { v.visit(*this); }
    void FuncDeclStmt::accept(ASTVisitor& v) { v.visit(*this); }
    void Program::accept(ASTVisitor& v) { v.visit(*this); }

}
