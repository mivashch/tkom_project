#include "ast.h"

namespace minilang::ast {

    void LiteralExpr::accept(ASTVisitor& v) { v.visit(*this); }
    void IdentifierExpr::accept(ASTVisitor& v) { v.visit(*this); }
    void UnaryExpr::accept(ASTVisitor& v) { v.visit(*this); }
    void BinaryExpr::accept(ASTVisitor& v) { v.visit(*this); }
    void CallExpr::accept(ASTVisitor& v) { v.visit(*this); }
    void LambdaExpr::accept(ASTVisitor& v) { v.visit(*this); }

    void ExprStmt::accept(ASTVisitor& v) { v.visit(*this); }
    void VarDeclStmt::accept(ASTVisitor& v) { v.visit(*this); }
    void AssignStmt::accept(ASTVisitor& v) { v.visit(*this); }
    void ReturnStmt::accept(ASTVisitor& v) { v.visit(*this); }
    void BlockStmt::accept(ASTVisitor& v) { v.visit(*this); }
    void IfStmt::accept(ASTVisitor& v) { v.visit(*this); }
    void ForStmt::accept(ASTVisitor& v) { v.visit(*this); }
    void FuncDeclStmt::accept(ASTVisitor& v) { v.visit(*this); }
    void Program::accept(ASTVisitor& v) { v.visit(*this); }

}
