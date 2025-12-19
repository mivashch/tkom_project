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


    static Token makeInt(long long v, Position p) {
        return Token(
            TokenKind::NumberInt,
            v,
            std::to_string(v),
            p
        );
    }

    static Token makeFloat(double v, Position p) {
        return Token(
            TokenKind::NumberFloat,
            v,
            std::to_string(v),
            p
        );
    }

    static Token makeString(std::string v, Position p) {
        return Token(
            TokenKind::String,
            v,
            "\"" + v + "\"",
            p
        );
    }

    static Token makeBool(bool v, Position p) {
        return Token(
            TokenKind::Bool,
            v,
            v ? "true" : "false",
            p
        );
    }


    static Token makeIdentifier(std::string name, Position p) {
        return Token(
            TokenKind::Identifier,
            name,
            name,
            p
        );
    }


    static Token makeKeyword(TokenKind kwKind, std::string lex, Position p) {
        return Token(
            kwKind,
            std::monostate{},
            std::move(lex),
            p
        );
    }


    static Token makeSimple(TokenKind kind, std::string lex, Position p) {
        return Token(
            kind,
            std::monostate{},
            std::move(lex),
            p
        );
    }

    static Token makeOperator(TokenKind kind, std::string lexeme, Position pos) {
        return Token(kind, std::monostate{}, std::move(lexeme), pos);
    }


    static Token makeEOF(Position p) {
        return Token(
            TokenKind::EndOfFile,
            std::monostate{},
            "",
            p
        );
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
