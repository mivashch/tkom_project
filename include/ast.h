#pragma once
#include <string>
#include <utility>
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
    Node(Position pos) : pos(pos) {}

    Node() = default;

    Position pos;
    virtual void accept(ASTVisitor &v) = 0;
};

struct Expr : Node {};

struct LiteralExpr : Expr {
    ::minilang::TokenValue value;
    LiteralExpr(TokenValue v, Position p)
    : value(std::move(v)) {pos = p; }
    void accept(ASTVisitor &v) override;
};

struct IdentifierExpr : Expr {
    std::string name;
    IdentifierExpr(std::string n, Position p)
    : name(std::move(n)) {pos = p;}
    void accept(ASTVisitor &v) override;
};

struct UnaryExpr : Expr {
    std::string op;
    std::unique_ptr<Expr> rhs;
    UnaryExpr(std::string op,
          std::unique_ptr<Expr> rhs,
          Position p)
    : op(std::move(op)), rhs(std::move(rhs)) {pos = p;}
    void accept(ASTVisitor &v) override;
};

struct BinaryExpr : Expr {
    std::string op;
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;
    BinaryExpr(std::string op,
      std::unique_ptr<Expr> lhs,
      std::unique_ptr<Expr> rhs,
      Position p)
: op(std::move(op)),lhs(std::move(lhs)), rhs(std::move(rhs)) {pos = p;}
    void accept(ASTVisitor &v) override;
};

struct CallExpr : Expr {
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> args;
    CallExpr(std::unique_ptr<Expr> callee,std::vector<std::unique_ptr<Expr>> args, Position p) :
    callee(std::move(callee)), args(std::move(args)) {pos = p;}
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
    AssignExpr(std::string target,
           std::unique_ptr<Expr> value,
           Position p)
    : target(std::move(target)),
      value(std::move(value)) {pos = p;}
    void accept(ASTVisitor &v) override;
};

struct Stmt : Node {
    Stmt() = default;
};

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
    VarDeclStmt( bool cnst, std::string name, std::unique_ptr<Expr> init, Position ps) : Stmt(), isConst(cnst),
        name(std::move(name)), init(std::move(init)) { pos = ps; }
};

struct ReturnStmt : Stmt {
    std::unique_ptr<Expr> value;
    ReturnStmt(std::unique_ptr<Expr> val, Position p)
    :  value(std::move(val)) {pos = p;}
    void accept(ASTVisitor &v) override;
};

struct BlockStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;
    explicit BlockStmt() = default;
    void accept(ASTVisitor &v) override;
};

struct IfStmt : Stmt {
    std::unique_ptr<Expr> cond;
    std::unique_ptr<BlockStmt> thenBlock;
    std::unique_ptr<BlockStmt> elseBlock;
    IfStmt(std::unique_ptr<Expr> cond,
       std::unique_ptr<BlockStmt> thenB,
       std::unique_ptr<BlockStmt> elseB,
       Position p)
    :
      cond(std::move(cond)),
      thenBlock(std::move(thenB)),
      elseBlock(std::move(elseB)) {pos = p;}
    void accept(ASTVisitor &v) override;
};

struct ForStmt : Stmt {
    std::unique_ptr<Stmt> initDecl;
    std::unique_ptr<Expr> initExpr;
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Expr> post;
    std::unique_ptr<BlockStmt> body;
    ForStmt(std::unique_ptr<Stmt> init,
        std::unique_ptr<Expr> initExpr,
        std::unique_ptr<Expr> cond,
        std::unique_ptr<Expr> post,
        std::unique_ptr<BlockStmt> body,
        Position p)
    :
      initDecl(std::move(init)),
      initExpr(std::move(initExpr)),
      cond(std::move(cond)),
      post(std::move(post)),
      body(std::move(body)) {}
    void accept(ASTVisitor &v) override;
};

struct FuncDeclStmt : Stmt {
    std::optional<std::string> returnType;
    std::string name;
    std::vector<std::pair<std::string, std::optional<std::string>>> params;
    std::unique_ptr<BlockStmt> body;
    FuncDeclStmt(std::string nm,
        std::optional<std::string> rType,std::vector<std::pair<std::string, std::optional<std::string>>> prms,
        std::unique_ptr<BlockStmt> bd, Position ps) : name(std::move(nm)), returnType(std::move(rType)), params(std::move(prms)), body(std::move(bd)) { pos = ps; }
    void accept(ASTVisitor &v) override;
};

struct Program : Node {
    explicit Program(std::vector<std::unique_ptr<Stmt>> statements);
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
