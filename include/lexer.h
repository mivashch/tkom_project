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

        bool isPunctuator(char c) const ;
        std::optional<Token> handleAmpersand(Position pos);
        std::optional<Token> handleEquals(Position pos);
        std::optional<Token> handlePipe(Position pos);
        std::optional<Token> handleBangLtGt(Position pos);
        std::optional<Token> handleArithmeticOp(Position pos);
        std::optional<Token> handlePunctuator(Position pos);
        std::optional<Token> handleUnknown(Position pos, char ch);


    private:
        std::unique_ptr<Source> src_;
        std::optional<Token> pushbackToken_;


        void skipWhitespaceAndComments();

        std::optional<Token> readIdentifierOrKeyword();

        std::optional<Token> readNumber();

        std::optional<Token> readString();

        Token makeToken(TokenKind kind, std::string lexeme, TokenValue value, Position pos);
        Token makeToken(TokenKind kind, std::string lexeme, Position pos);



    };
}
