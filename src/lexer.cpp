#include "lexer.h"
#include "source.h"
#include "token.h"

#include <cctype>
#include <stdexcept>
#include <climits>
#include <algorithm>

namespace minilang {


 TokenKind Lexer::keywordKind(const std::string& s) {
    if (s == "fun")    return TokenKind::KwFun;
    if (s == "return") return TokenKind::KwReturn;
    if (s == "if")     return TokenKind::KwIf;
    if (s == "else")   return TokenKind::KwElse;
    if (s == "for")    return TokenKind::KwFor;
    if (s == "const")  return TokenKind::KwConst;

    if (s == "int")    return TokenKind::KwInt;
    if (s == "float")  return TokenKind::KwFloat;
    if (s == "str")    return TokenKind::KwStr;
    if (s == "bool")   return TokenKind::KwBool;

    return TokenKind::Unknown;
}

bool Lexer::isPunctuator(char c) const {

    switch (c) {
        case '(':
        case ')':
        case '{':
        case '}':
        case ',':
        case ';':
        case ':':
            return true;
        default:
            return false;
    }
}


Lexer::Lexer(std::unique_ptr<Source> src)
    : src_(std::move(src)) {}

Token Lexer::nextToken() {
    skipWhitespaceAndComments();
    Position pos = src_->getPosition();

    if (src_->peek() == -1) {
        return Token::makeOperator(TokenKind::EndOfFile, "", pos);
    }

 if (auto token = readIdentifierOrKeyword()) return *token;
 if (auto token = readNumber()) return *token;
 if (auto token = readString()) return *token;
 if (auto token = handleAmpersand(pos)) return *token;
 if (auto token = handleEquals(pos)) return *token;
 if (auto token = handlePipe(pos)) return *token;
 if (auto token = handleBangLtGt(pos)) return *token;
 if (auto token = handleArithmeticOp(pos)) return *token;
 if (auto token = handlePunctuator(pos)) return *token;

 char unexpectedChar = static_cast<char>(src_->get());
 return Token::makeOperator(
     TokenKind::Unknown,
     std::string(1, unexpectedChar),
     pos
 );

}

std::optional<Token> Lexer::readIdentifierOrKeyword() {
    int c = src_->peek();
    if (!std::isalpha(c) && c != '_') return std::nullopt;

    Position pos = src_->getPosition();
    std::string buf;

    buf.push_back(static_cast<char>(src_->get()));

    while (true) {
        int p = src_->peek();
        if (p == -1) break;
        char ch = static_cast<char>(p);
        if (!std::isalnum(ch) && ch != '_') break;
        buf.push_back(static_cast<char>(src_->get()));
    }

    if (buf == "true")  return Token::makeBool(true, pos);
    if (buf == "false") return Token::makeBool(false, pos);

    TokenKind kw = keywordKind(buf);
    if (kw != TokenKind::Unknown) {
        return Token::makeOperator(kw, buf, pos);
    }

    return Token::makeIdentifier(buf, pos);
}

std::optional<Token> Lexer::readNumber() {
    if (!std::isdigit(src_->peek())) return std::nullopt;

    Position pos = src_->getPosition();

    long long intVal = 0;
    double floatVal = 0.0;
    bool isFloat = false;
    double fracMul = 0.1;

    while (true) {
        int p = src_->peek();
        if (p == -1) break;

        char ch = static_cast<char>(p);

        if (std::isdigit(ch)) {
            src_->get();
            int digit = ch - '0';

            if (!isFloat) {
                if (LLONG_MAX / 10 < intVal)
                    throw std::runtime_error("Integer literal overflow");
                intVal = intVal * 10 + digit;
            } else {
                floatVal += digit * fracMul;
                fracMul *= 0.1;
            }
            continue;
        }

        if (ch == '.') {
            if (isFloat) break;
            isFloat = true;
            floatVal = static_cast<double>(intVal);
            src_->get();
            continue;
        }

        break;
    }

    if (isFloat)
        return Token::makeFloat(floatVal, pos);
    else
        return Token::makeInt(intVal, pos);
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
            if (e == -1) throw std::runtime_error("Unterminated escape");

            switch (static_cast<char>(e)) {
                case 'n': buf.push_back('\n'); break;
                case 't': buf.push_back('\t'); break;
                case '\\': buf.push_back('\\'); break;
                case '"': buf.push_back('"'); break;
                default: buf.push_back(static_cast<char>(e)); break;
            }
        } else {
            buf.push_back(ch);
        }
    }

    return Token::makeString(buf, pos);
}

std::optional<Token> Lexer::handleAmpersand(Position pos) {
    if (src_->peek() != '&') return std::nullopt;
    src_->get();

    if (src_->peek() == '*') {
        src_->get();
        if (src_->peek() == '&') {
            src_->get();
            return Token::makeOperator(TokenKind::OpRefStarRef, "&*&", pos);
        }
    }

    if (src_->peek() == '&') {
        src_->get();
        return Token::makeOperator(TokenKind::OpAnd, "&&", pos);
    }

    return Token::makeOperator(TokenKind::Unknown, "&", pos);
}

std::optional<Token> Lexer::handleEquals(Position pos) {
    if (src_->peek() != '=') return std::nullopt;
    src_->get();

    if (src_->peek() == '=') {
        src_->get();
        return Token::makeOperator(TokenKind::OpEq, "==", pos);
    }

    if (src_->peek() == '>') {
        src_->get();
        if (src_->peek() == '>') {
            src_->get();
            return Token::makeOperator(TokenKind::OpDoubleArrow, "=>>", pos);
        }
        return Token::makeOperator(TokenKind::OpArrow, "=>", pos);
    }

    return Token::makeOperator(TokenKind::OpAssign, "=", pos);
}

std::optional<Token> Lexer::handlePipe(Position pos) {
    if (src_->peek() != '|') return std::nullopt;
    src_->get();

    if (src_->peek() == '|') {
        src_->get();
        return Token::makeOperator(TokenKind::OpOr, "||", pos);
    }

    return Token::makeOperator(TokenKind::Unknown, "|", pos);
}

std::optional<Token> Lexer::handleBangLtGt(Position pos) {
    char ch = static_cast<char>(src_->peek());
    if (ch != '!' && ch != '<' && ch != '>') return std::nullopt;
    src_->get();

    if (ch == '!') {
        if (src_->peek() == '=') {
            src_->get();
            return Token::makeOperator(TokenKind::OpNotEq, "!=", pos);
        }
    }

    if (ch == '<') {
        if (src_->peek() == '=') {
            src_->get();
            return Token::makeOperator(TokenKind::OpLessEq, "<=", pos);
        }
        return Token::makeOperator(TokenKind::OpLess, "<", pos);
    }

    if (ch == '>') {
        if (src_->peek() == '=') {
            src_->get();
            return Token::makeOperator(TokenKind::OpGreaterEq, ">=", pos);
        }
        return Token::makeOperator(TokenKind::OpGreater, ">", pos);
    }

    return std::nullopt;
}

std::optional<Token> Lexer::handleArithmeticOp(Position pos) {
    char ch = static_cast<char>(src_->peek());
    if (ch != '+' && ch != '-' && ch != '*' && ch != '/' && ch != '%')
        return std::nullopt;

    src_->get();

    switch (ch) {
        case '+': return Token::makeOperator(TokenKind::OpPlus, "+", pos);
        case '-': return Token::makeOperator(TokenKind::OpMinus, "-", pos);
        case '*': return Token::makeOperator(TokenKind::OpMul, "*", pos);
        case '/': return Token::makeOperator(TokenKind::OpDiv, "/", pos);
        case '%': return Token::makeOperator(TokenKind::OpMod, "%", pos);
    }

    return std::nullopt;
}


std::optional<Token> Lexer::handlePunctuator(Position pos) {
    char ch = static_cast<char>(src_->peek());
    if (!isPunctuator(ch)) return std::nullopt;

    src_->get();

    switch (ch) {
        case '(': return Token::makeOperator(TokenKind::LParen, "(", pos);
        case ')': return Token::makeOperator(TokenKind::RParen, ")", pos);
        case '{': return Token::makeOperator(TokenKind::LBrace, "{", pos);
        case '}': return Token::makeOperator(TokenKind::RBrace, "}", pos);
        case ',': return Token::makeOperator(TokenKind::Comma, ",", pos);
        case ';': return Token::makeOperator(TokenKind::Semicolon, ";", pos);
        case ':': return Token::makeOperator(TokenKind::Colon, ":", pos);
    }

    return std::nullopt;
}


void Lexer::skipWhitespaceAndComments() {
    while (true) {
        int c = src_->peek();
        if (c == -1) return;

        char ch = static_cast<char>(c);

        if (std::isspace(static_cast<unsigned char>(ch))) {
            src_->get();
            continue;
        }

        if (ch == '/') {
            src_->get();
            if (src_->peek() == '/') {
                while (true) {
                    int r = src_->get();
                    if (r == -1 || r == '\n') break;
                }
                continue;
            }
            if (src_->peek() == '*') {
                src_->get();
                while (true) {
                    int r = src_->get();
                    if (r == -1) throw std::runtime_error("Unterminated comment");
                    if (r == '*' && src_->peek() == '/') {
                        src_->get();
                        break;
                    }
                }
                continue;
            }
            src_->unget();
        }

        break;
    }
}

}
