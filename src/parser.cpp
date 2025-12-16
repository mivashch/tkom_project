#include <sstream>
#include <memory>
#include <vector>
#include <optional>

#include "ast.h"
#include "parser.h"
#include "lexer.h"
#include "token.h"

using namespace minilang;
using namespace minilang::ast;


Parser::Parser(Lexer &lex) : lex_(lex) {
    cur_ = lex_.nextToken();
}

Token Parser::next() {
    cur_ = lex_.nextToken();
    return cur_;
}



bool Parser::match(TokenKind k, const std::string *lexeme) {
    if (cur_.getKind() != k) return false;
    if (lexeme && cur_.lexeme != *lexeme) return false;
    next();
    return true;
}

bool Parser::expect(TokenKind k, const std::string *lexeme) {
    if (cur_.getKind() != k || (lexeme && cur_.lexeme != *lexeme)) {
        std::ostringstream ss;
        if (lexeme) ss << "Unexpected token, expected kind " << static_cast<int>(k) << "('" << *lexeme << "')";
        else ss << "Unexpected token, expected kind " << static_cast<int>(k);
        errorAt(cur_, ss.str());
        return false;
    }
    next();
    return true;
}

void Parser::errorAt(const Token &t, const std::string &msg) {
    std::ostringstream ss;
    ss << "ParseError [" << t.pos.line << ":" << t.pos.column << "]: " << msg << " (got '" << t.lexeme << "')";
    lastError_ = ss.str();
}

std::unique_ptr<Program> Parser::parseProgram() {
    auto prog = std::make_unique<Program>();
    prog->pos = cur_.pos;
    while (true) {
        if (cur_.getKind() == TokenKind::EndOfFile) break;
        auto s = parseStatement();
        if (lastError_) {
            return nullptr;
        }
        prog->stmts.push_back(std::move(s));
    }
    return prog;
}

// statement = var_decl | assign | func_decl | expr_stmt | if_stmt | for_stmt | ";"
std::unique_ptr<Stmt> Parser::parseStatement() {
    static const std::string semicolon = ";";

    if (cur_.getKind() == TokenKind::Keyword) {
        if (cur_.lexeme == "fun")    return parseFuncDecl();
        if (cur_.lexeme == "if")     return parseIf();
        if (cur_.lexeme == "for")    return parseFor();
        if (cur_.lexeme == "return") return parseReturn();
        if (cur_.lexeme == "const")  return parseVarDecl();
    }

    if (cur_.getKind() == TokenKind::Punctuator && cur_.lexeme == "{")
        return parseBlock();

    if (cur_.getKind() == TokenKind::Punctuator && cur_.lexeme == ";") {
        next();
        return std::make_unique<ExprStmt>();
    }


        auto expr = parseAssign();
        if (!expr) return nullptr;



    if (!expect(TokenKind::Punctuator, &semicolon)) return nullptr;

    auto st = std::make_unique<ExprStmt>();
    st->expr = std::move(expr);
    st->pos = st->expr->pos;
    return st;
}

// var_decl = [var_mod] , identifier , "=" , func_op_expr , ";" ;
std::unique_ptr<Stmt> Parser::parseVarDecl() {
    static const std::string semicolon = ";";

    bool isConst = false;
    if (cur_.getKind() == TokenKind::Keyword && cur_.lexeme == "const") {
        isConst = true;
        next();
    }
    Token id = cur_;
    if (id.getKind() != TokenKind::Identifier) {
        errorAt(id, "Expected identifier in variable declaration");
        return nullptr;
    }
    next();
    std::string name = id.lexeme;

    if (!expect(TokenKind::OpAssign, nullptr)) return nullptr;

    auto init = parseFuncOpExpr();
    if (!init) return nullptr;
    if (!expect(TokenKind::Punctuator, &semicolon)) return nullptr;

    auto v = std::make_unique<VarDeclStmt>();
    v->isConst = isConst;
    v->name = name;
    v->init = std::move(init);
    v->pos = id.pos;
    return v;
}

// assign = identifier , "=" , func_op_expr , ";" ;
std::unique_ptr<Expr> Parser::parseAssign() {
    auto left = parseFuncOpExpr();
    if (!left) return nullptr;

    if (cur_.getKind() == TokenKind::OpAssign) {
        Token t = cur_;
        next();

        auto rhs = parseAssign();
        if (!rhs) return nullptr;

        auto id = dynamic_cast<IdentifierExpr*>(left.get());
        if (!id) {
            errorAt(t, "Left-hand side of assignment must be identifier");
            return nullptr;
        }

        auto ae = std::make_unique<AssignExpr>();
        ae->target = id->name;
        ae->value = std::move(rhs);
        ae->pos = t.pos;
        return ae;
    }
    return left;
}

// func_decl = "fun" , type_spec ,  identifier  , "(" , [ param_list ] , ")"  , body ;
std::unique_ptr<Stmt> Parser::parseFuncDecl() {
    static const std::string leftParen = "(";
    static const std::string rightParen = ")";

    Token kw = cur_;
    if (!(kw.getKind() == TokenKind::Keyword && kw.lexeme == "fun")) {
        errorAt(kw, "Expected 'fun' to start function declaration");
        return nullptr;
    }
    next();

    std::optional<std::string> retType;
    if (cur_.getKind() == TokenKind::Keyword &&
        (cur_.lexeme == "int" || cur_.lexeme == "float" || cur_.lexeme == "str" || cur_.lexeme == "bool" || cur_.lexeme == "fun"))
    {
        retType = cur_.lexeme;
        next();
    } else if (cur_.getKind() == TokenKind::Identifier) {
        retType = cur_.lexeme;
        next();
    }

    Token nameTok = cur_;
    if (nameTok.getKind() != TokenKind::Identifier) {
        errorAt(nameTok, "Expected function name");
        return nullptr;
    }
    next();
    std::string fname = nameTok.lexeme;

    if (!expect(TokenKind::Punctuator, &leftParen)) return nullptr;
    auto params = parseParamList();
    if (!expect(TokenKind::Punctuator, &rightParen)) return nullptr;
    auto body = parseBlock();
    if (!body) return nullptr;

    auto f = std::make_unique<FuncDeclStmt>();
    f->name = fname;
    f->returnType = retType;
    f->params = std::move(params);
    f->body = std::move(body);
    f->pos = nameTok.pos;
    return f;
}

std::vector<std::pair<std::string,std::optional<std::string>>> Parser::parseParamList() {
    std::vector<std::pair<std::string,std::optional<std::string>>> out;
    if (cur_.getKind() == TokenKind::Punctuator && cur_.lexeme == ")") return out;

    while (true) {
        bool isConst = false;
        if (cur_.getKind() == TokenKind::Keyword && cur_.lexeme == "const") {
            isConst = true;
            next();
        }
        if (cur_.getKind() != TokenKind::Identifier) { errorAt(cur_, "Expected parameter name"); return {}; }
        std::string name = cur_.lexeme;
        next();

        std::optional<std::string> typ;
        if (cur_.getKind() == TokenKind::Punctuator && cur_.lexeme == ":") {
            next();
            if (cur_.getKind() == TokenKind::Keyword || cur_.getKind() == TokenKind::Identifier) {
                typ = cur_.lexeme;
                next();
            } else { errorAt(cur_, "Expected type name after ':'"); return {}; }
        }

        out.emplace_back(name, typ);

        if (cur_.getKind() == TokenKind::Punctuator && cur_.lexeme == ",") { next(); continue; }
        break;
    }
    return out;
}

std::unique_ptr<BlockStmt> Parser::parseBlock() {
    static const std::string leftBrace = "{";
    static const std::string rightBrace = "}";

    if (!expect(TokenKind::Punctuator, &leftBrace)) return nullptr;
    auto blk = std::make_unique<BlockStmt>();
    while (true) {
        if (cur_.getKind() == TokenKind::Punctuator && cur_.lexeme == rightBrace) {
            next();
            break;
        }
        if (cur_.getKind() == TokenKind::EndOfFile ) {
            errorAt(cur_, "Expected '}'");
            break;
        };
        auto st = parseStatement();
        blk->stmts.push_back(std::move(st));
    }
    return blk;
}

// return_stmt = "return" , func_op_expr , ";"
std::unique_ptr<Stmt> Parser::parseReturn() {
    static const std::string semicolon = ";";

    Token kw = cur_;
    if (!(kw.getKind() == TokenKind::Keyword && kw.lexeme == "return")) {
        errorAt(kw, "Expected 'return'");
        return nullptr;
    }
    next();
    auto e = parseFuncOpExpr();
    if (!e) return nullptr;
    if (!expect(TokenKind::Punctuator, &semicolon)) return nullptr;
    auto r = std::make_unique<ReturnStmt>();
    r->value = std::move(e);
    r->pos = kw.pos;
    return r;
}

// if_stmt = "if" , "(" , func_op_expr , ")" , "{" , { statement } , "}" , [ "else" , "{" , { statement } , "}" ] ;
std::unique_ptr<Stmt> Parser::parseIf() {
    Token kw = cur_;
    if (!(kw.getKind() == TokenKind::Keyword && kw.lexeme == "if")) {
        errorAt(kw, "Expected 'if'");
        return nullptr;
    }
    next();

    static const std::string leftParen = "(";
    static const std::string rightParen = ")";

    if (!expect(TokenKind::Punctuator, &leftParen)) return nullptr;
    auto cond = parseFuncOpExpr();
    if (!cond) return nullptr;
    if (!expect(TokenKind::Punctuator, &rightParen)) return nullptr;
    auto thenB = parseBlock();
    if (!thenB) return nullptr;

    std::unique_ptr<BlockStmt> elseB = nullptr;
    if (cur_.getKind() == TokenKind::Keyword && cur_.lexeme == "else") {
        next();
        elseB = parseBlock();
        if (!elseB) return nullptr;
    }

    auto s = std::make_unique<IfStmt>();
    s->cond = std::move(cond);
    s->thenBlock = std::move(thenB);
    s->elseBlock = std::move(elseB);
    s->pos = kw.pos;
    return s;
}

// for_stmt = "for" , "(" , [ assign ] , ";" , func_op_expr , ";" , [ assign ] , ")" , "{" , { statement } , "}"
std::unique_ptr<Stmt> Parser::parseFor() {
    Token kw = cur_;
    if (!(kw.getKind() == TokenKind::Keyword && kw.lexeme == "for")) {
        errorAt(kw, "Expected 'for'");
        return nullptr;
    }
    next();

    static const std::string lpar = "(";
    static const std::string rpar = ")";
    static const std::string semi = ";";

    if (!expect(TokenKind::Punctuator, &lpar)) return nullptr;

    std::unique_ptr<Stmt> initDecl = nullptr;
    std::unique_ptr<Expr> initExpr = nullptr;


    if (!(cur_.getKind() == TokenKind::Punctuator && cur_.lexeme == semi)) {
        if (cur_.getKind() == TokenKind::Keyword && cur_.lexeme == "const") {
            initDecl = parseVarDecl();
            if (!initDecl) return nullptr;
        } else {
            initExpr = parseAssign();
            if (!initExpr) return nullptr;
            if (!expect(TokenKind::Punctuator, &semi)) return nullptr;
        }
    } else {
        next();
    }

    std::unique_ptr<Expr> cond = nullptr;
    if (!(cur_.getKind() == TokenKind::Punctuator && cur_.lexeme == semi)) {
        cond = parseFuncOpExpr();
        if (!cond) return nullptr;
    }
    if (!expect(TokenKind::Punctuator, &semi)) return nullptr;

    std::unique_ptr<Expr> post = nullptr;
    if (!(cur_.getKind() == TokenKind::Punctuator && cur_.lexeme == rpar)) {
        post = parseAssign();
        if (!post) return nullptr;
    }

    if (!expect(TokenKind::Punctuator, &rpar)) return nullptr;

    auto body = parseBlock();
    if (!body) return nullptr;

    auto f = std::make_unique<ForStmt>();
    f->initDecl = std::move(initDecl);
    f->initExpr = std::move(initExpr);
    f->cond = std::move(cond);
    f->post = std::move(post);
    f->body = std::move(body);
    f->pos = kw.pos;
    return f;
}


// func_op_expr = logic_expr , { ("&*&" | "=>>") , logic_expr } ;
std::unique_ptr<Expr> Parser::parseFuncOpExpr() {
    auto left = parseLogicExpr();
    if (!left) return nullptr;
    while (true) {
        if (cur_.getKind() == TokenKind::OpRefStarRef || cur_.getKind() == TokenKind::OpDoubleArrow) {
            Token t = cur_;
            next();
            auto right = parseLogicExpr();
            if (!right) { errorAt(t, "Expected expression after function-operator"); return nullptr; }
            auto be = std::make_unique<BinaryExpr>();
            be->op = t.lexeme;
            be->lhs = std::move(left);
            be->rhs = std::move(right);
            be->pos = t.pos;
            left = std::move(be);
        } else break;
    }
    return left;
}

// logic_expr = comp_expr , { ("&&" | "||") , comp_expr } ;
std::unique_ptr<Expr> Parser::parseLogicExpr() {
    auto left = parseCompExpr();
    if (!left) return nullptr;
    while (true) {
        if (cur_.getKind() == TokenKind::OpAnd || cur_.getKind() == TokenKind::OpOr) {
            Token t = cur_;
            next();
            auto right = parseCompExpr();
            if (!right) { errorAt(t, "Expected expression after logical operator"); return nullptr; }
            auto be = std::make_unique<BinaryExpr>();
            be->op = t.lexeme;
            be->lhs = std::move(left);
            be->rhs = std::move(right);
            be->pos = t.pos;
            left = std::move(be);
        } else break;
    }
    return left;
}

// comp_expr = add_expr , [ ("==" | "!=" | "<" | "<=" | ">" | ">=") , add_expr ] ;
std::unique_ptr<Expr> Parser::parseCompExpr() {
    auto left = parseAddExpr();
    if (!left) return nullptr;
    if (cur_.getKind() == TokenKind::OpEq ||
        cur_.getKind() == TokenKind::OpNotEq ||
        cur_.getKind() == TokenKind::OpLess ||
        cur_.getKind() == TokenKind::OpLessEq ||
        cur_.getKind() == TokenKind::OpGreater ||
        cur_.getKind() == TokenKind::OpGreaterEq)
    {
        Token t = cur_;
        next();
        auto right = parseAddExpr();
        if (!right) { errorAt(t, "Expected right-hand side for comparison"); return nullptr; }
        auto be = std::make_unique<BinaryExpr>();
        be->op = t.lexeme;
        be->lhs = std::move(left);
        be->rhs = std::move(right);
        be->pos = t.pos;
        return be;
    }
    return left;
}

// add_expr = mul_expr , { ( "+" | "-" ) , mul_expr } ;
std::unique_ptr<Expr> Parser::parseAddExpr() {
    auto left = parseMulExpr();
    if (!left) return nullptr;
    while (true) {
        if (cur_.getKind() == TokenKind::OpPlus || cur_.getKind() == TokenKind::OpMinus) {
            Token t = cur_;
            next();
            auto right = parseMulExpr();
            if (!right) { errorAt(t, "Expected operand after + or -"); return nullptr; }
            auto be = std::make_unique<BinaryExpr>();
            be->op = t.lexeme;
            be->lhs = std::move(left);
            be->rhs = std::move(right);
            be->pos = t.pos;
            left = std::move(be);
        } else break;
    }
    return left;
}

// mul_expr = unary_expr , { ( "*" | "/" | "%" ) , unary_expr } ;
std::unique_ptr<Expr> Parser::parseMulExpr() {
    auto left = parseUnaryExpr();
    if (!left) return nullptr;
    while (true) {
        if (cur_.getKind() == TokenKind::OpMul || cur_.getKind() == TokenKind::OpDiv || cur_.getKind() == TokenKind::OpMod) {
            Token t = cur_;
            next();
            auto right = parseUnaryExpr();
            if (!right) { errorAt(t, "Expected operand after * / %"); return nullptr; }
            auto be = std::make_unique<BinaryExpr>();
            be->op = t.lexeme;
            be->lhs = std::move(left);
            be->rhs = std::move(right);
            be->pos = t.pos;
            left = std::move(be);
        } else break;
    }
    return left;
}

// unary_expr = [ "-" | "!" ] , call_or_primary ;
std::unique_ptr<Expr> Parser::parseUnaryExpr() {
    if (cur_.getKind() == TokenKind::OpMinus || cur_.getKind() == TokenKind::OpNot) {
        Token t = cur_;
        next();
        auto rhs = parseUnaryExpr();
        if (!rhs) { errorAt(t, "Unary operator requires operand"); return nullptr; }
        auto ue = std::make_unique<UnaryExpr>();
        ue->op = t.lexeme;
        ue->rhs = std::move(rhs);
        ue->pos = t.pos;
        return ue;
    }
    return parseCallOrPrimary();
}

// call_or_primary = primary , { "(" , [ arg_list ] , ")" } ;
std::unique_ptr<Expr> Parser::parseCallOrPrimary() {
    auto prim = parsePrimary();
    if (!prim) return nullptr;
    while (cur_.getKind() == TokenKind::Punctuator && cur_.lexeme == "(") {
        Token t = cur_;
        next();
        auto args = parseArgList();
        static const std::string rightParen = ")";
        if (!expect(TokenKind::Punctuator, &rightParen)) return nullptr;
        auto call = std::make_unique<CallExpr>();
        call->callee = std::move(prim);
        call->args = std::move(args);
        call->pos = t.pos;
        prim = std::move(call);
    }
    return prim;
}

// arg_list = func_op_expr , { "," , func_op_expr } ;
std::vector<std::unique_ptr<Expr>> Parser::parseArgList() {
    std::vector<std::unique_ptr<Expr>> out;
    static const std::string rightParen = ")";
    if (cur_.getKind() == TokenKind::Punctuator && cur_.lexeme == rightParen) return out;
    while (true) {
        auto e = parseFuncOpExpr();
        if (!e) { errorAt(cur_, "Expected argument expression"); return {}; }
        out.push_back(std::move(e));
        if (cur_.getKind() == TokenKind::Punctuator && cur_.lexeme == ",") {
            next();
            continue;
        }
        break;
    }
    return out;
}

// primary = literal | identifier | "(" , func_op_expr , ")"
std::unique_ptr<Expr> Parser::parsePrimary() {
    static const std::string rightParen = ")";

    if (cur_.getKind() == TokenKind::NumberInt || cur_.getKind() == TokenKind::NumberFloat ||
        cur_.getKind() == TokenKind::String || cur_.getKind() == TokenKind::Bool) {
        Token t = cur_;
        next();
        auto lit = makeLiteralFromToken(t);
        lit->pos = t.pos;
        return lit;
    }
    if (cur_.getKind() == TokenKind::Identifier) {
        Token t = cur_;
        next();
        auto id = std::make_unique<IdentifierExpr>();
        id->name = t.lexeme;
        id->pos = t.pos;
        return id;
    }
    if (cur_.getKind() == TokenKind::Punctuator && cur_.lexeme == "(") {
        next();
        auto e = parseFuncOpExpr();
        if (!e) return nullptr;
        if (!expect(TokenKind::Punctuator, &rightParen)) return nullptr;
        return e;
    }
    errorAt(cur_, "Expected primary expression");
    return nullptr;
}

std::unique_ptr<LiteralExpr> Parser::makeLiteralFromToken(const Token &t) {
    auto lit = std::make_unique<LiteralExpr>();
    lit->value = t.getValue();
    lit->pos = t.pos;
    return lit;
}
