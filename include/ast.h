#pragma once
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <variant>
#include "token.h"
#include "tools.h"

namespace minilang::ast {

struct ASTVisitor;

using Position = ::minilang::Position;

struct Node {
    virtual ~Node() = default;
    Position pos;
    virtual void accept(ASTVisitor &v) = 0;
};

struct Expr : Node {};

struct LiteralExpr : Expr {
    ::minilang::TokenValue value;
    void accept(ASTVisitor &v) override;
};

struct IdentifierExpr : Expr {
    std::string name;
    void accept(ASTVisitor &v) override;
};

struct UnaryExpr : Expr {
    std::string op;
    std::unique_ptr<Expr> rhs;
    void accept(ASTVisitor &v) override;
};

struct BinaryExpr : Expr {
    std::string op;
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;
    void accept(ASTVisitor &v) override;
};

struct CallExpr : Expr {
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> args;
    void accept(ASTVisitor &v) override;
};

struct LambdaExpr : Expr {
    std::vector<std::string> params;
    std::unique_ptr<Expr> bodyExpr;
    void accept(ASTVisitor &v) override;
};

struct AssignExpr : Expr {
    std::string target;
    std::unique_ptr<Expr> value;
    void accept(ASTVisitor &v) override;
};

struct Stmt : Node {};

struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expr;
    void accept(ASTVisitor &v) override;
};

struct VarDeclStmt : Stmt {
    bool isConst = false;
    std::optional<std::string> typeName;
    std::string name;
    std::unique_ptr<Expr> init;
    void accept(ASTVisitor &v) override;
};

struct ReturnStmt : Stmt {
    std::unique_ptr<Expr> value;
    void accept(ASTVisitor &v) override;
};

struct BlockStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;
    void accept(ASTVisitor &v) override;
};

struct IfStmt : Stmt {
    std::unique_ptr<Expr> cond;
    std::unique_ptr<BlockStmt> thenBlock;
    std::unique_ptr<BlockStmt> elseBlock;
    void accept(ASTVisitor &v) override;
};

struct ForStmt : Stmt {
    std::unique_ptr<Stmt> initDecl;
    std::unique_ptr<Expr> initExpr;
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Expr> post;
    std::unique_ptr<BlockStmt> body;
    void accept(ASTVisitor &v) override;
};

struct FuncDeclStmt : Stmt {
    std::optional<std::string> returnType;
    std::string name;
    std::vector<std::pair<std::string, std::optional<std::string>>> params;
    std::unique_ptr<BlockStmt> body;
    void accept(ASTVisitor &v) override;
};

struct Program : Node {
    Program(std::vector<std::unique_ptr<Stmt>> statements);
    std::vector<std::unique_ptr<Stmt>> stmts;
    void accept(ASTVisitor &v) override;
};

struct ASTVisitor {
    virtual ~ASTVisitor() = default;
    virtual void visit(LiteralExpr&) = 0;
    virtual void visit(IdentifierExpr&) = 0;
    virtual void visit(UnaryExpr&) = 0;
    virtual void visit(BinaryExpr&) = 0;
    virtual void visit(CallExpr&) = 0;
    virtual void visit(LambdaExpr&) = 0;

    virtual void visit(ExprStmt&) = 0;
    virtual void visit(VarDeclStmt&) = 0;
    virtual void visit(AssignExpr&) = 0;
    virtual void visit(ReturnStmt&) = 0;
    virtual void visit(BlockStmt&) = 0;
    virtual void visit(IfStmt&) = 0;
    virtual void visit(ForStmt&) = 0;
    virtual void visit(FuncDeclStmt&) = 0;
    virtual void visit(Program&) = 0;
};

}
