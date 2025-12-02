#pragma once
#include <string>
#include <variant>
#include "tools.h"


namespace minilang {
    enum class TokenKind {
        EndOfFile,
        Identifier,
        NumberInt,
        NumberFloat,
        String,
        Bool,
        Keyword,

        OpAnd,          // &&
        OpRefStarRef,   // &*&
        OpAssign,       // =
        OpEq,           // ==
        OpArrow,        // =>
        OpDoubleArrow,  // =>>
        OpOr,           // ||
        OpNot,          // !
        OpNotEq,        // !=
        OpLess,         // <
        OpLessEq,       // <=
        OpGreater,      // >
        OpGreaterEq,    // >=
        OpPlus,         // +
        OpMinus,        // -
        OpMul,          // *
        OpDiv,          // /
        OpMod,          // %

        Punctuator,
        Comment,
        Unknown
    };







    using TokenValue = std::variant<std::monostate, long long, double, std::string, bool>;


    struct Token {
        std::string lexeme;
        Position pos;

        void setValue(TokenValue val) { value = val; }
        void setKind(TokenKind kd) { kind = kd; }
        TokenValue getValue() const { return value; }
        TokenKind getKind() const { return kind; }

    private:
        TokenKind kind;
        TokenValue value;
    };
}
