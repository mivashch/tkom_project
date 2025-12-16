#pragma once
#include "lexer.h"
#include "ast.h"
#include <optional>
#include <vector>
#include <string>

namespace minilang {

using namespace ast;

class Parser {
public:
    explicit Parser(Lexer &lex);

    std::unique_ptr<Program> parseProgram();

    std::optional<std::string> lastError() const { return lastError_; }

private:
    Lexer &lex_;

    std::optional<std::string> lastError_;

    Token cur_;

    Token peekNext();
    Token next();
    bool match(TokenKind k, const std::string *lexeme = nullptr);
    bool expect(TokenKind k, const std::string *lexeme = nullptr);

    void errorAt(const Token &t, const std::string &msg);

    // statement = var_decl | assign | func_decl | expr_stmt | if_stmt | for_stmt | ";"
    std::unique_ptr<Stmt> parseStatement();

    // var_decl = [var_mod] identifier "=" func_op_expr ";"
    std::unique_ptr<Stmt> parseVarDecl();

    // assign = identifier "=" func_op_expr ";"
    std::unique_ptr<Expr> parseAssign();

    // func_decl = "fun" , type_spec ,  identifier  , "(" , [ param_list ] , ")"  , body ;
    std::unique_ptr<Stmt> parseFuncDecl();

    // param_list/parameter parsers
    std::vector<std::pair<std::string,std::optional<std::string>>> parseParamList();

    // body = "{" , { statement } , [ return_stmt ] , "}"
    std::unique_ptr<BlockStmt> parseBlock();

    // return_stmt = "return" , func_op_expr , ";"
    std::unique_ptr<Stmt> parseReturn();

    // if_stmt = "if" , "(" , func_op_expr , ")" , "{" , { statement } , "}" , [ "else" , "{" , { statement } , "}" ] ;
    std::unique_ptr<Stmt> parseIf();

    // for_stmt = "for" , "(" , [ assign ] , ";" , func_op_expr , ";" , [ assign ] , ")" , "{" , { statement } , "}"
    std::unique_ptr<Stmt> parseFor();

    std::unique_ptr<Expr> parseFuncOpExpr(); // func_op_expr = logic_expr , { ("&*&" | "=>>") , logic_expr } ;
    std::unique_ptr<Expr> parseLogicExpr();  // logic_expr = comp_expr , { ("&&" | "||") , comp_expr } ;
    std::unique_ptr<Expr> parseCompExpr();   // comp_expr = add_expr , [ ("==" | "!=" | "<" | "<=" | ">" | ">=") , add_expr ] ;
    std::unique_ptr<Expr> parseAddExpr();    // add_expr = mul_expr , { ( "+" | "-" ) , mul_expr } ;
    std::unique_ptr<Expr> parseMulExpr();    // mul_expr = unary_expr , { ( "*" | "/" | "%" ) , unary_expr } ;
    std::unique_ptr<Expr> parseUnaryExpr();  // unary_expr = [ "-" | "!" ] , call_or_primary ;
    std::unique_ptr<Expr> parseCallOrPrimary(); // call_or_primary = primary , { "(" , [ arg_list ] , ")" } ;
    std::unique_ptr<Expr> parsePrimary();    // primary = literal | identifier | "(" , func_op_expr , ")"

    std::vector<std::unique_ptr<Expr>> parseArgList();

    std::unique_ptr<LiteralExpr> makeLiteralFromToken(const Token &t);
};

}
