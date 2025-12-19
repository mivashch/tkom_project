#pragma once

#include "lexer.h"
#include "ast.h"

#include <memory>
#include <optional>
#include <vector>
#include <string>

namespace minilang {

using namespace ast;

class Parser {
public:
    explicit Parser(Lexer& lex);

    std::unique_ptr<Program> parseProgram();
    [[nodiscard]] std::optional<std::string> lastError() const { return lastError_; }



private:
    Lexer& lex_;

    std::optional<std::string> lastError_;


    Token cur_;

    Token next();
    bool match(TokenKind kind);
    bool expect(TokenKind kind);

    void errorAt(const Token& t, const std::string& msg);


    // statement = var_decl | assign | func_decl | expr_stmt | if_stmt | for_stmt | ";"
    std::unique_ptr<Stmt> parseStatement();

    // var_decl = "const" identifier "=" func_op_expr ";"
    std::unique_ptr<Stmt> parseVarDecl();

    // assign = identifier "=" func_op_expr
    std::unique_ptr<Expr> parseAssign();

    // func_decl = "fun" type_spec identifier "(" [ param_list ] ")" body
    std::unique_ptr<Stmt> parseFuncDecl();

    // param_list = parameter { "," parameter }
    std::vector<std::pair<std::string, std::optional<std::string>>> parseParamList();

    // body = "{" { statement } "}"
    std::unique_ptr<BlockStmt> parseBlock();

    std::unique_ptr<BlockStmt> parseElseBlock();


    // return_stmt = "return" func_op_expr ";"
    std::unique_ptr<Stmt> parseReturn();

    // if_stmt = "if" "(" func_op_expr ")" block [ "else" block ]
    std::unique_ptr<Stmt> parseIf();

    // for_stmt = "for" "(" [assign|var_decl] ";" func_op_expr ";" [assign] ")" block
    std::unique_ptr<Stmt> parseFor();

    // func_op_expr = logic_expr { ("&*&" | "=>>") logic_expr }
    std::unique_ptr<Expr> parseFuncOpExpr();

    // logic_expr = comp_expr { ("&&" | "||") comp_expr }
    std::unique_ptr<Expr> parseLogicExpr();

    // comp_expr = add_expr [ ("==" | "!=" | "<" | "<=" | ">" | ">=") add_expr ]
    std::unique_ptr<Expr> parseCompExpr();

    // add_expr = mul_expr { ("+" | "-") mul_expr }
    std::unique_ptr<Expr> parseAddExpr();

    // mul_expr = unary_expr { ("*" | "/" | "%") unary_expr }
    std::unique_ptr<Expr> parseMulExpr();

    // unary_expr = [ "-" | "!" ] call_or_primary
    std::unique_ptr<Expr> parseUnaryExpr();

    // call_or_primary = primary { "(" [ arg_list ] ")" }
    std::unique_ptr<Expr> parseCallOrPrimary();

    // primary = literal | identifier | "(" func_op_expr ")"
    std::unique_ptr<Expr> parsePrimary();

    // arg_list = func_op_expr { "," func_op_expr }
    std::vector<std::unique_ptr<Expr>> parseArgList();

    std::unique_ptr<LiteralExpr> makeLiteralFromToken(const Token& t);
};

}
