#include "lexer.h"
#include "source.h"
#include <cctype>
#include <stdexcept>
#include <climits>
#include <iomanip>
#include <memory>
#include <algorithm>

namespace minilang {
    std::array<std::string, 13> keywords = {
        "fun", "return", "if", "else", "for", "const", "mut",
        "true", "false", "int", "float", "str", "bool"
    };

    constexpr std::array<char, 6> punctuators = {'(', ')', '{', '}', ',', ';'};


    Lexer::Lexer(std::unique_ptr<Source> src) : src_(std::move(src)), pushbackToken_(std::nullopt) {
    }


    Token Lexer::peekToken() {
        if (pushbackToken_) return *pushbackToken_;
        auto tok = nextToken();
        pushbackToken_ = tok;
        return *pushbackToken_;
    }

    Token Lexer::nextToken() {
        if (pushbackToken_) {
            Token t = *pushbackToken_;
            pushbackToken_.reset();
            return t;
        }

        skipWhitespaceAndComments();
        Position pos = src_->getPosition();

        if (src_->peek() == -1) {
            return makeToken(TokenKind::EndOfFile, "", pos);
        }

        if (auto t = readIdentifierOrKeyword()) return *t;
        if (auto t = readNumber()) return *t;
        if (auto t = readString()) return *t;
        if (auto t = handleAmpersand(pos)) return *t;
        if (auto t = handleEquals(pos)) return *t;
        if (auto t = handlePipe(pos)) return *t;
        if (auto t = handleBangLtGt(pos)) return *t;
        if (auto t = handleArithmeticOp(pos)) return *t;
        if (auto t = handlePunctuator(pos)) return *t;

        char ch = static_cast<char>(src_->get());
        return makeToken(TokenKind::Unknown, std::string(1, ch), pos);
    }

    std::optional<Token> Lexer::readIdentifierOrKeyword() {
        int c = src_->peek();
        if (!isalpha(c) && c != '_') return std::nullopt;

        Position pos = src_->getPosition();
        std::string buf;

        buf += static_cast<char>(src_->get());

        while (true) {
            int p = src_->peek();
            if (p == -1) break;
            char ch = static_cast<char>(p);
            if (!isalnum(ch) && ch != '_') break;
            buf += static_cast<char>(src_->get());
        }

        if (buf == "true" || buf == "false") {
            bool v = (buf == "true");
            return makeToken(TokenKind::Bool, buf, TokenValue(v), pos);
        }

        if (std::find(keywords.begin(), keywords.end(), buf) != keywords.end()) {
            return makeToken(TokenKind::Keyword, buf, pos);
        }

        return makeToken(TokenKind::Identifier, buf, pos);
    }

    std::optional<Token> Lexer::readNumber() {
        int c = src_->peek();
        if (!isdigit(c)) return std::nullopt;

        Position pos = src_->getPosition();

        long long intVal = 0;
        double floatVal = 0.0;
        bool isFloat = false;
        double fracMul = 0.1;
        char ch=' ';

        while (true) {
            int p = src_->peek();
            if (p == -1) break;

            ch = static_cast<char>(p);

            if (isdigit(ch)) {
                int digit = ch - '0';
                src_->get();

                if (!isFloat) {
                    if (LLONG_MAX - intVal < digit) {
                        throw std::runtime_error("Overflow!");
                    }
                    intVal = intVal * 10 + digit;
                } else {
                    floatVal += digit * fracMul;
                    fracMul *= 0.1;
                }

                continue;
            }

            if (ch == '.') {
                if (isFloat) {
                    return makeToken(TokenKind::Unknown, std::string(1, ch), pos);
                }
                isFloat = true;
                floatVal = intVal;
                ch = src_->get();
                if (!isdigit(src_->peek())) {
                    return  makeToken(TokenKind::Unknown,  std::to_string(intVal)+ std::string(1, ch), pos);
                }
                continue;
            }

            if (isalpha(ch)) {
                return  makeToken(TokenKind::Unknown,  isFloat ? std::to_string(floatVal) : std::to_string(intVal) + std::string(1, src_->get()), pos);
            }

            break;
        }


        if (!isFloat)
            return makeToken(TokenKind::NumberInt,std::to_string(intVal), intVal, pos);
        else
            return makeToken(TokenKind::NumberFloat, std::to_string(floatVal), floatVal, pos);
    }


    std::optional<Token> Lexer::readString() {
        if (src_->peek() != '"') return std::nullopt;

        Position pos = src_->getPosition();
        src_->get();

        std::string buf;

        while (true) {
            int p = src_->get();
            if (p == -1) throw std::runtime_error("Unterminated string literal");
            char ch = static_cast<char>(p);

            if (ch == '"') break;

            if (ch == '\\') {
                int e = src_->get();
                if (e == -1) throw std::runtime_error("Unterminated escape in string");
                char esc = static_cast<char>(e);
                switch (esc) {
                    case 'n': buf.push_back('\n');
                        break;
                    case 't': buf.push_back('\t');
                        break;
                    case '\\': buf.push_back('\\');
                        break;
                    case '"': buf.push_back('"');
                        break;
                    default: buf.push_back(esc);
                        break;
                }
                continue;
            }

            buf.push_back(ch);
        }

        return makeToken(TokenKind::String, buf, TokenValue(buf), pos);
    }

    std::optional<Token> Lexer::handleAmpersand(Position pos) {
        if (src_->peek() != '&') return std::nullopt;

        src_->get();

        int c2 = src_->peek();
        if (c2 == '*') {
            src_->get();
            int c3 = src_->peek();
            if (c3 == '&') {
                src_->get();
                return makeToken(TokenKind::OpRefStarRef, "&*&", pos);
            }
            return makeToken(TokenKind::Unknown, std::string(1, c2), pos);
        }

        if (c2 == '&') {
            src_->get();
            return makeToken(TokenKind::OpAnd, "&&", pos);
        }

        return makeToken(TokenKind::Unknown, std::string(1, c2), pos);
    }

    std::optional<Token> Lexer::handleEquals(Position pos) {
        if (src_->peek() != '=') return std::nullopt;

        src_->get();
        int c2 = src_->peek();

        if (c2 == '>') {
            src_->get();
            int c3 = src_->peek();
            if (c3 == '>') {
                src_->get();
                return makeToken(TokenKind::OpDoubleArrow, "=>>", pos);
            }
            return makeToken(TokenKind::OpArrow, "=>", pos);
        }

        if (c2 == '=') {
            src_->get();
            return makeToken(TokenKind::OpEq, "==", pos);
        }

        if (isalpha(src_->peek()) || isdigit(src_->peek()) || isspace(src_->peek()) || (src_->peek() == -1)) {
            return makeToken(TokenKind::OpAssign, "=", pos);
        }
        return makeToken(TokenKind::Unknown, std::string(1, c2) + std::string(1, src_->get()), pos);
    }

    std::optional<Token> Lexer::handlePipe(Position pos) {
        if (src_->peek() != '|') return std::nullopt;

        char c = src_->get();
        if (src_->peek() == '|') {
            src_->get();
            return makeToken(TokenKind::OpOr, "||", pos);
        }
        return makeToken(TokenKind::Unknown, std::string(1, c), pos);
    }

    std::optional<Token> Lexer::handleBangLtGt(Position pos) {
        int c = src_->peek();
        if (c != '!' && c != '<' && c != '>') return std::nullopt;

        char ch = static_cast<char>(src_->get());

        switch (ch) {
            case '!':
                if (src_->peek() == '=') {
                    src_->get();
                    return makeToken(TokenKind::OpNotEq, "!=", pos);
                }
                return makeToken(TokenKind::OpNot, "!", pos);

            case '<':
                if (src_->peek() == '=') {
                    src_->get();
                    return makeToken(TokenKind::OpLessEq, "<=", pos);
                }
                return makeToken(TokenKind::OpLess, "<", pos);

            case '>':
                if (src_->peek() == '=') {
                    src_->get();
                    return makeToken(TokenKind::OpGreaterEq, ">=", pos);
                }
                return makeToken(TokenKind::OpGreater, ">", pos);
        }
        return std::nullopt;
    }

    std::optional<Token> Lexer::handleArithmeticOp(Position pos) {
        char ch = src_->peek();
        if (ch != '+' && ch != '-' && ch != '*' && ch != '/' && ch != '%')
            return std::nullopt;
        ch = static_cast<char>(src_->get());
        if (src_->peek() == '/') {
            return std::nullopt;
        }

        switch (ch) {
            case '+': return makeToken(TokenKind::OpPlus, "+", pos);
            case '-': return makeToken(TokenKind::OpMinus, "-", pos);
            case '*': return makeToken(TokenKind::OpMul, "*", pos);
            case '/': return makeToken(TokenKind::OpDiv, "/", pos);
            case '%': return makeToken(TokenKind::OpMod, "%", pos);
        }

        return std::nullopt;
    }

    std::optional<Token> Lexer::handlePunctuator(Position pos) {
        char ch = src_->peek();
        if (!isPunctuator(ch)) return std::nullopt;

        src_->get();
        return makeToken(TokenKind::Punctuator, std::string(1, ch), pos);
    }


    bool Lexer::isPunctuator(char c) const {
        return std::find(punctuators.begin(), punctuators.end(), c) != punctuators.end();
    }

    void Lexer::skipWhitespaceAndComments() {
        while (true) {
            int c = src_->peek();
            if (c == -1) return;
            char ch = static_cast<char>(c);
            if (isspace((unsigned char) ch)) {
                src_->get();
                continue;
            }
            if (ch == '/') {
                src_->get();
                int n = src_->peek();
                if (n == '/') {
                    while (true) {
                        int r = src_->get();
                        if (r == -1 || r == '\n') break;
                    }
                    continue;
                } else if (n == '*') {
                    src_->get();
                    while (true) {
                        int r = src_->get();
                        if (r == -1) throw std::runtime_error("Unterminated block comment");
                        if (r == '*' && src_->peek() == '/') {
                            src_->get();
                            break;
                        }
                    }
                    continue;
                } else {
                    src_->unget();
                    break;
                }
            }
            break;
        }
    }


    Token Lexer::makeToken(TokenKind kind, std::string lexeme, TokenValue value, Position pos) {
        Token t;
        t.setKind(kind);
        t.lexeme = std::move(lexeme);
        t.setValue(std::move(value));
        t.pos = pos;
        return t;
    }

    Token Lexer::makeToken(TokenKind kind, std::string lexeme, Position pos) {
        Token t;
        t.setKind(kind);
        t.lexeme = std::move(lexeme);
        t.setValue(std::monostate{});
        t.pos = pos;
        return t;
    }
}
