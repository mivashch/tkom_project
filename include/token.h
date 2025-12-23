#pragma once
#include <string>
#include <variant>
#include "tools.h"

namespace minilang {



enum class TokenKind {
    EndOfFile,
    Unknown,

    Identifier,
    NumberInt,
    NumberFloat,
    String,
    Bool,

    KwFun,
    KwReturn,
    KwIf,
    KwElse,
    KwFor,
    KwConst,

    KwInt,
    KwFloat,
    KwStr,
    KwBool,

    OpAssign,        // =
    OpEq,            // ==
    OpNotEq,         // !=

    OpAnd,           // &&
    OpOr,            // ||

    OpLess,          // <
    OpLessEq,        // <=
    OpGreater,       // >
    OpGreaterEq,     // >=

    OpPlus,          // +
    OpMinus,         // -
    OpMul,           // *
    OpDiv,           // /
    OpMod,           // %

    OpRefStarRef,    // &*&
    OpArrow,         // =>
    OpDoubleArrow,   // =>>

    LParen,          // (
    RParen,          // )
    LBrace,          // {
    RBrace,          // }
    Comma,           // ,
    Semicolon,       // ;
    Colon            // :
};


using TokenValue = std::variant<
    std::monostate,
    long long,
    double,
    std::string,
    bool
>;


struct Token {
public:
    TokenKind getKind() const { return kind; }
    const TokenValue& getValue() const { return value; }
    const std::string& getLexeme() const { return lexeme; }
    Position getPos() const { return pos; }


    Token(TokenKind k, std::string lex, Position p)
        : kind(k), value(std::monostate{}), lexeme(std::move(lex)), pos(p) {}

    Token(long long v, Position p)
        : kind(TokenKind::NumberInt),
          value(v),
          lexeme(std::to_string(v)),
          pos(p) {}

    Token(double v, Position p)
        : kind(TokenKind::NumberFloat),
          value(v),
          lexeme(std::to_string(v)),
          pos(p) {}

    Token(std::string v, Position p)
        : kind(TokenKind::String),
          value(v),
          lexeme("\"" + v + "\""),
          pos(p) {}

    Token(bool v, Position p)
        : kind(TokenKind::Bool),
          value(v),
          lexeme(v ? "true" : "false"),
          pos(p) {}

    static Token Identifier(std::string name, Position p) {
        return Token(TokenKind::Identifier, std::move(name), p);
    }

private:
    TokenKind kind;
    TokenValue value;
    std::string lexeme;
    Position pos;

    Token(TokenKind k, TokenValue v, std::string lx, Position p)
        : kind(k),
          value(std::move(v)),
          lexeme(std::move(lx)),
          pos(p) {}
};

}
