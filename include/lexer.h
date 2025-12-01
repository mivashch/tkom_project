#pragma once
#include "token.h"
#include "source.h"
#include <optional>
#include <string>
#include <functional>


namespace minilang {
    class Lexer {
    public:
        explicit Lexer(std::unique_ptr<Source> src);

        Token nextToken();

        Token peekToken();

        void reset();

    private:
        std::unique_ptr<Source> src_;
        std::optional<Token> pushbackToken_;


        void skipWhitespaceAndComments();

        Token readIdentifierOrKeyword();

        Token readNumber();

        Token readString();

        Token makeToken(TokenKind kind, std::string lexeme, TokenValue value, size_t line, size_t column);


        bool isIdentifierStart(char c) const;

        bool isIdentifierPart(char c) const;
    };
}
