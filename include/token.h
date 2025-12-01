#pragma once
#include <string>
#include <variant>
#include <cstdint>


namespace minilang {
    enum class TokenKind {
        EndOfFile,
        Identifier,
        NumberInt,
        NumberFloat,
        String,
        Bool,
        Keyword,
        Operator,
        Punctuator,
        Comment,
        Unknown
    };


    struct Position {
        size_t line = 1;
        size_t column = 1;
    };


    using TokenValue = std::variant<std::monostate, long long, double, std::string, bool>;


    struct Token {
        TokenKind kind;
        std::string lexeme;
        TokenValue value;
        Position pos;
    };
}
