#include <sstream>
#include <memory>
#include <vector>
#include <optional>
#include <iostream>

#include "ast.h"
#include "parser.h"
#include "lexer.h"
#include "token.h"

using namespace minilang;
using namespace minilang::ast;

Parser::Parser(Lexer& lex)
    : lex_(lex)
    , cur_(lex_.nextToken())
{
}


Token Parser::next() {
    cur_ = lex_.nextToken();
    return cur_;
}

bool Parser::match(TokenKind k) {
    if (cur_.getKind() != k) return false;
    next();
    return true;
}

inline std::string toString(TokenKind k) {
    switch (k) {
        case TokenKind::EndOfFile:   return "EndOfFile";
        case TokenKind::Identifier:  return "Identifier";

        case TokenKind::NumberInt:   return "NumberInt";
        case TokenKind::NumberFloat: return "NumberFloat";
        case TokenKind::String:      return "String";
        case TokenKind::Bool:        return "Bool";

        case TokenKind::KwFun:       return "fun";
        case TokenKind::KwReturn:    return "return";
        case TokenKind::KwIf:        return "if";
        case TokenKind::KwElse:      return "else";
        case TokenKind::KwFor:       return "for";
        case TokenKind::KwConst:     return "const";

        case TokenKind::KwInt:       return "int";
        case TokenKind::KwFloat:     return "float";
        case TokenKind::KwStr:       return "str";
        case TokenKind::KwBool:      return "bool";

        case TokenKind::OpAssign:    return "=";
        case TokenKind::OpEq:        return "==";
        case TokenKind::OpNotEq:     return "!=";
        case TokenKind::OpLess:      return "<";
        case TokenKind::OpLessEq:    return "<=";
        case TokenKind::OpGreater:   return ">";
        case TokenKind::OpGreaterEq: return ">=";

        case TokenKind::OpPlus:      return "+";
        case TokenKind::OpMinus:     return "-";
        case TokenKind::OpMul:       return "*";
        case TokenKind::OpDiv:       return "/";
        case TokenKind::OpMod:       return "%";

        case TokenKind::OpAnd:        return "&&";
        case TokenKind::OpOr:         return "||";
        case TokenKind::OpRefStarRef: return "&*&";
        case TokenKind::OpArrow:      return "=>";
        case TokenKind::OpDoubleArrow:return "=>>";

        case TokenKind::LParen:      return "(";
        case TokenKind::RParen:      return ")";
        case TokenKind::LBrace:      return "{";
        case TokenKind::RBrace:      return "}";
        case TokenKind::Comma:       return ",";
        case TokenKind::Semicolon:   return ";";
        case TokenKind::Colon:       return ":";

        case TokenKind::Unknown:     return "Unknown";
    }
    return "<invalid TokenKind>";
}


bool Parser::expect(TokenKind k) {
    if (cur_.getKind() != k) {
        std::ostringstream ss;
        ss << "Expected token " << toString(k);
        errorAt(cur_, ss.str());
        return false;
    }
    next();
    return true;
}

[[noreturn]]
void Parser::errorAt(const Token& t, const std::string& msg) {
    std::ostringstream ss;
    ss << "ParseError [" << t.getPos().line << ":" << t.getPos().column
       << "]: " << msg
       << " (got '" << t.getLexeme() << "')";
    throw ParseError(t.getPos(), ss.str());
}


std::unique_ptr<Program> Parser::parseProgram() {
    std::vector<std::unique_ptr<Stmt>> stmts;

    while (auto stmt = parseStatement()) {
        stmts.push_back(std::move(stmt));
    }

    expect(TokenKind::EndOfFile);
    return std::make_unique<Program>(std::move(stmts));
}


std::unique_ptr<Stmt> Parser::parseStatement() {
    if (auto s = parseFuncDecl())   return s;
    if (auto s = parseIf())         return s;
    if (auto s = parseFor())        return s;
    if (auto s = parseReturn())     return s;
    if (auto s = parseVarDecl())    return s;
    if (auto s = parseBlock())  return s;
    if (auto s = parseExpression()) return s;

    return nullptr;
}

std::unique_ptr<Stmt> Parser::parseExpression() {
    if (match(TokenKind::Semicolon)) {
        next();
        return std::make_unique<ExprStmt>();
    }
    if (match(TokenKind::EndOfFile)) return nullptr;

    auto expr = parseAssign();

    expect(TokenKind::Semicolon);

    auto st = std::make_unique<ExprStmt>();
    st->expr = std::move(expr);
    st->pos = st->expr->pos;
    return st;

}


std::unique_ptr<Stmt> Parser::parseVarDecl() {
    if (cur_.getKind() != TokenKind::KwConst) return nullptr;


    Token id = cur_;
    expect(TokenKind::Identifier);

    expect(TokenKind::OpAssign);

    auto init = parseFuncOpExpr();
    expect(TokenKind::Semicolon);

    auto v = std::make_unique<VarDeclStmt>();
    v->isConst = true;
    v->name = id.getLexeme();
    v->init = std::move(init);
    v->pos = id.getPos();
    return v;
}


std::unique_ptr<Expr> Parser::parseAssign() {
    auto id = cur_;
    auto left = parseFuncOpExpr();

    if (cur_.getKind() != TokenKind::OpAssign)
        return left;

    if (!left || !dynamic_cast<IdentifierExpr *>(left.get()))
        errorAt(id, "Left side of assignment must be identifier");



    Token assignTok = cur_;
    next();

    auto rhs = parseAssign();
    if (!rhs)
        errorAt(assignTok, "Expected expression after '='");

    if (id.getKind() != TokenKind::Identifier)
        errorAt(assignTok, "Left side of assignment must be identifier");

    auto a = std::make_unique<AssignExpr>();
    a->target = id.getLexeme();
    a->value = std::move(rhs);
    a->pos = assignTok.getPos();
    return a;
}



std::unique_ptr<Stmt> Parser::parseFuncDecl() {
    if (cur_.getKind() != TokenKind::KwFun) return nullptr;
    expect(TokenKind::KwFun);

    std::optional<std::string> retType;
    if (cur_.getKind() == TokenKind::KwInt ||
        cur_.getKind() == TokenKind::KwFloat ||
        cur_.getKind() == TokenKind::KwStr ||
        cur_.getKind() == TokenKind::KwBool ||
        cur_.getKind() == TokenKind::KwFun) {
        retType = cur_.getLexeme();
        next();
    }

    Token nameTok = cur_;
    expect(TokenKind::Identifier);

    expect(TokenKind::LParen);
    auto params = parseParamList();
    expect(TokenKind::RParen);

    auto body = parseBlock();

    auto f = std::make_unique<FuncDeclStmt>();
    f->name = nameTok.getLexeme();
    f->returnType = retType;
    f->params = std::move(params);
    f->body = std::move(body);
    f->pos = nameTok.getPos();
    return f;
}

std::vector<std::pair<std::string, std::optional<std::string>>>
Parser::parseParamList() {
    std::vector<std::pair<std::string, std::optional<std::string>>> params;

    if (cur_.getKind() == TokenKind::RParen)
        return params;

    while (cur_.getKind() != TokenKind::RParen) {

        bool isConst = match(TokenKind::KwConst);

        Token nameTok = cur_;
        expect(TokenKind::Identifier);

        std::optional<std::string> type;
        if (match(TokenKind::Colon)) {
            type = cur_.getLexeme();
            next();
        }

        params.emplace_back(nameTok.getLexeme(), type);

        if (match(TokenKind::Comma))
            continue;

        break;
    }

    return params;
}


std::unique_ptr<BlockStmt> Parser::parseBlock() {
    if (cur_.getKind() != TokenKind::LBrace) return nullptr;

    expect(TokenKind::LBrace);

    auto blk = std::make_unique<BlockStmt>();

    while (cur_.getKind() != TokenKind::RBrace && cur_.getKind() != TokenKind::EndOfFile) {
        blk->stmts.push_back(parseStatement());
    }

    expect(TokenKind::RBrace);
    return blk;
}

std::unique_ptr<Stmt> Parser::parseReturn() {
    if (cur_.getKind() != TokenKind::KwReturn) return nullptr;

    Token t = cur_;
    expect(TokenKind::KwReturn);

    std::unique_ptr<Expr> value = nullptr;

    if (cur_.getKind() != TokenKind::Semicolon) {
        value = parseFuncOpExpr();
        if (!value)
            errorAt(cur_, "Expected expression after 'return'");
    }

    expect(TokenKind::Semicolon);

    auto r = std::make_unique<ReturnStmt>();
    r->value = std::move(value);
    r->pos = t.getPos();
    return r;
}


std::unique_ptr<BlockStmt> Parser::parseElseBlock() {
    if (match(TokenKind::KwElse))
        return parseBlock();

    return nullptr;
}


std::unique_ptr<Stmt> Parser::parseIf() {
    if (cur_.getKind() != TokenKind::KwIf) return nullptr;

    Token t = cur_;
    expect(TokenKind::KwIf);

    expect(TokenKind::LParen);
    auto cond = parseFuncOpExpr();
    expect(TokenKind::RParen);

    auto thenB = parseBlock();

    auto elseB = parseElseBlock();

    auto s = std::make_unique<IfStmt>();
    s->cond = std::move(cond);
    s->thenBlock = std::move(thenB);
    s->elseBlock = std::move(elseB);
    s->pos = t.getPos();
    return s;
}

std::unique_ptr<Stmt> Parser::parseFor() {
    if (cur_.getKind() != TokenKind::KwFor) return nullptr;

    Token t = cur_;
    expect(TokenKind::KwFor);

    expect(TokenKind::LParen);

    std::unique_ptr<Stmt> initDecl;
    std::unique_ptr<Expr> initExpr;

    if (cur_.getKind() == TokenKind::KwConst)
        initDecl = parseVarDecl();
    else if (cur_.getKind() != TokenKind::Semicolon) {
        initExpr = parseAssign();
        expect(TokenKind::Semicolon);
    } else {
        next();
    }

    auto cond = (cur_.getKind() == TokenKind::Semicolon)
                    ? nullptr
                    : parseFuncOpExpr();
    expect(TokenKind::Semicolon);

    auto post = (cur_.getKind() == TokenKind::RParen)
                    ? nullptr
                    : parseAssign();

    expect(TokenKind::RParen);

    auto body = parseBlock();
    if (!body) {
        errorAt(cur_, "Expected body of function");

    }

    auto f = std::make_unique<ForStmt>();
    f->initDecl = std::move(initDecl);
    f->initExpr = std::move(initExpr);
    f->cond = std::move(cond);
    f->post = std::move(post);
    f->body = std::move(body);
    f->pos = t.getPos();
    return f;
}

std::unique_ptr<Expr> Parser::parseFuncOpExpr() {
    auto left = parseLogicExpr();

    while (cur_.getKind() == TokenKind::OpRefStarRef ||
           cur_.getKind() == TokenKind::OpDoubleArrow) {
        Token t = cur_;
        next();

        auto right = parseLogicExpr();

        auto b = std::make_unique<BinaryExpr>();
        b->op = t.getLexeme();
        b->lhs = std::move(left);
        b->rhs = std::move(right);
        b->pos = t.getPos();
        left = std::move(b);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseLogicExpr() {
    auto left = parseCompExpr();

    while (cur_.getKind() == TokenKind::OpAnd ||
           cur_.getKind() == TokenKind::OpOr) {
        Token t = cur_;
        next();

        auto right = parseCompExpr();

        auto b = std::make_unique<BinaryExpr>();
        b->op = t.getLexeme();
        b->lhs = std::move(left);
        b->rhs = std::move(right);
        b->pos = t.getPos();
        left = std::move(b);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseCompExpr() {
    auto left = parseAddExpr();

    switch (cur_.getKind()) {
        case TokenKind::OpEq:
        case TokenKind::OpNotEq:
        case TokenKind::OpLess:
        case TokenKind::OpLessEq:
        case TokenKind::OpGreater:
        case TokenKind::OpGreaterEq: {
            Token t = cur_;
            next();

            auto right = parseAddExpr();

            auto b = std::make_unique<BinaryExpr>();
            b->op = t.getLexeme();
            b->lhs = std::move(left);
            b->rhs = std::move(right);
            b->pos = t.getPos();
            return b;
        }
        default:
            return left;
    }
}

std::unique_ptr<Expr> Parser::parseAddExpr() {
    auto left = parseMulExpr();

    while (cur_.getKind() == TokenKind::OpPlus ||
           cur_.getKind() == TokenKind::OpMinus) {
        Token t = cur_;
        next();

        auto right = parseMulExpr();

        auto b = std::make_unique<BinaryExpr>();
        b->op = t.getLexeme();
        b->lhs = std::move(left);
        b->rhs = std::move(right);
        b->pos = t.getPos();
        left = std::move(b);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseMulExpr() {
    auto left = parseUnaryExpr();

    while (cur_.getKind() == TokenKind::OpMul ||
           cur_.getKind() == TokenKind::OpDiv ||
           cur_.getKind() == TokenKind::OpMod) {
        Token t = cur_;
        next();

        auto right = parseUnaryExpr();

        auto b = std::make_unique<BinaryExpr>();
        b->op = t.getLexeme();
        b->lhs = std::move(left);
        b->rhs = std::move(right);
        b->pos = t.getPos();
        left = std::move(b);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseUnaryExpr() {
    if (cur_.getKind() == TokenKind::OpMinus ){
        Token t = cur_;
        next();

        auto rhs = parseUnaryExpr();

        auto u = std::make_unique<UnaryExpr>();
        u->op = t.getLexeme();
        u->rhs = std::move(rhs);
        u->pos = t.getPos();
        return u;
    }
    return parseCallOrPrimary();
}

std::unique_ptr<Expr> Parser::parseCallOrPrimary() {
    auto prim = parsePrimary();

    while (match(TokenKind::LParen)) {
        auto args = parseArgList();
        expect(TokenKind::RParen);

        auto c = std::make_unique<CallExpr>();
        c->callee = std::move(prim);
        c->args = std::move(args);
        c->pos = c->callee->pos;
        prim = std::move(c);
    }
    return prim;
}

std::vector<std::unique_ptr<Expr>> Parser::parseArgList() {
    std::vector<std::unique_ptr<Expr>> args;

    if (cur_.getKind() == TokenKind::RParen)
        return args;

    while (true) {
        args.push_back(parseFuncOpExpr());
        if (!match(TokenKind::Comma))
            break;
    }
    return args;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    Token t = cur_;

    switch (cur_.getKind()) {
        case TokenKind::NumberInt:
        case TokenKind::NumberFloat:
        case TokenKind::String:
        case TokenKind::Bool: {
            next();
            return makeLiteralFromToken(t);
        }
        case TokenKind::Identifier: {
            next();
            auto id = std::make_unique<IdentifierExpr>();
            id->name = t.getLexeme();
            id->pos = t.getPos();
            return id;
        }
        case TokenKind::LParen:
            next();
            {
                auto e = parseFuncOpExpr();
                expect(TokenKind::RParen);
                return e;
            }
        default:
            errorAt(cur_, "Expected primary expression");
            return nullptr;
    }
}

std::unique_ptr<LiteralExpr> Parser::makeLiteralFromToken(const Token& t) {
    auto l = std::make_unique<LiteralExpr>();
    l->value = t.getValue();
    l->pos = t.getPos();
    return l;
}
