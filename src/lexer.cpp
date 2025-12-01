#include "lexer.h"
#include "source.h"
#include <cctype>
#include <stdexcept>
#include <unordered_set>
#include <iomanip>
#include <memory>
namespace minilang {
    static const std::unordered_set<std::string> keywords = {
        "fun", "return", "if", "else", "for", "const", "mut",
        "true", "false", "int", "float", "str", "bool"
    };

    static const std::unordered_set<char> punctuators = {'(', ')', '{', '}', ',', ';'};

    Lexer::Lexer(std::unique_ptr<Source> src) : src_(std::move(src)), pushbackToken_(std::nullopt) {
    }

    void Lexer::reset() {
        pushbackToken_.reset();
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
        size_t line = src_->line();
        size_t col = src_->column();
        int c = src_->peek();
        if (c == -1) {
            return makeToken(TokenKind::EndOfFile, "", std::monostate{}, line, col);
        }
        char ch = static_cast<char>(c);
        if (isalpha(ch) || ch == '_') {
            return readIdentifierOrKeyword();
        }
        if (isdigit(ch)) {
            return readNumber();
        }
        if (ch == '"') {
            return readString();
        }
        if (ch == '&') {
            src_->get();
            int c2 = src_->peek();
            if (c2 == '*') {
                src_->get();
                int c3 = src_->peek();
                if (c3 == '&') {
                    src_->get();
                    return makeToken(TokenKind::Operator, "&*&", std::monostate{}, line, col);
                }
                return makeToken(TokenKind::Unknown, "&*", std::monostate{}, line, col);
            }
            if (c2 == '&') {
                src_->get();
                return makeToken(TokenKind::Operator, "&&", std::monostate{}, line, col);
            }
            return makeToken(TokenKind::Unknown, "&", std::monostate{}, line, col);
        }
        if (ch == '=') {
            src_->get();
            int c2 = src_->peek();
            if (c2 == '>') {
                src_->get();
                int c3 = src_->peek();
                if (c3 == '>') {
                    src_->get();
                    return makeToken(TokenKind::Operator, "=>>", std::monostate{}, line, col);
                }
                return makeToken(TokenKind::Operator, "=>", std::monostate{}, line, col);
            }
            if (src_->peek() == '=') {
                src_->get();
                return makeToken(TokenKind::Operator, "==", std::monostate{}, line, col);
            }
            if (isalpha(src_->peek()) || isdigit(src_->peek()) || isspace(src_->peek()) ||( src_->peek() == -1)) {
                return makeToken(TokenKind::Operator, "=", std::monostate{}, line, col);
            }
            return makeToken(TokenKind::Unknown, std::string(1,ch) + std::string(1,src_->get()) ,std::monostate{}, line, col);

        }
        if (ch == '|') {
            src_->get();
            int c2 = src_->peek();
            src_->get();
            if (c2 == '|') return makeToken(TokenKind::Operator, "|/", std::monostate{}, line, col);
        }
        if (ch == '!' || ch == '<' || ch == '>') {
            src_->get();
            int c2 = src_->peek();
            std::string op(1, ch);
            if (c2 == '=') {
                src_->get();
                op.push_back('=');
            }
            return makeToken(TokenKind::Operator, op, std::monostate{}, line, col);
        }
        if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%') {
            src_->get();
            std::string s(1, ch);
            if (isalpha(src_->peek()) || isdigit(src_->peek()) || isspace(src_->peek()) || src_->peek() == -1) {
                return makeToken(TokenKind::Operator, s, std::monostate{}, line, col);
            }
            return makeToken(TokenKind::Unknown, std::string(1,ch) + std::string(1,src_->get()) ,std::monostate{}, line, col);

        }
        if (punctuators.count(ch)) {
            src_->get();
            std::string s(1, ch);
            return makeToken(TokenKind::Punctuator, s, std::monostate{}, line, col);
        }

        src_->get();
        std::string s(1, ch);
        return makeToken(TokenKind::Unknown, s, std::monostate{}, line, col);
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

    bool Lexer::isIdentifierStart(char c) const { return std::isalpha((unsigned char) c) || c == '_'; }
    bool Lexer::isIdentifierPart(char c) const { return std::isalnum((unsigned char) c) || c == '_'; }

    Token Lexer::readIdentifierOrKeyword() {
        size_t line = src_->line();
        size_t col = src_->column();
        std::string buf;
        int c = src_->get();
        buf.push_back(static_cast<char>(c));
        while (true) {
            int p = src_->peek();
            if (p == -1) break;
            char ch = static_cast<char>(p);
            if (!isIdentifierPart(ch)) break;
            buf.push_back((char) src_->get());
        }
        if (buf == "true" || buf == "false") {
            bool v = (buf == "true");
            return makeToken(TokenKind::Bool, buf, TokenValue(v), line, col);
        }
        if (keywords.count(buf)) {
            return makeToken(TokenKind::Keyword, buf, std::monostate{}, line, col);
        }
        return makeToken(TokenKind::Identifier, buf, std::monostate{}, line, col);
    }

    Token Lexer::readNumber() {
        size_t line = src_->line();
        size_t col = src_->column();
        std::string buf;
        bool seenDot = false;
        bool wrong = false;
        while (true) {
            int p = src_->peek();
            if (p == -1) break;
            char ch = static_cast<char>(p);
            if (isalpha(ch)) {
                wrong = true;
                buf.push_back(ch);
                src_->get();
                continue;
            }
            if (ch == '.' && !seenDot) {
                seenDot = true;
                buf.push_back(ch);
                src_->get();
                continue;
            }
            else if((ch == '.' || isalpha(ch)) && seenDot) {
                wrong = true;
                buf.push_back(ch);
                src_->get();
                continue;
            }
            if (!isdigit((unsigned char) ch) && (ch !='.')) break;
            buf.push_back((char) src_->get());
        }
        try {
            if (wrong) {
                double d = std::stod(buf);
                return makeToken(TokenKind::Unknown, buf, TokenValue(d), line, col);
            }
            else if (seenDot) {
                double d = std::stod(buf);
                return makeToken(TokenKind::NumberFloat, buf, TokenValue(d), line, col);
            } else {
                long long v = std::stoll(buf);
                return makeToken(TokenKind::NumberInt, buf, TokenValue(v), line, col);
            }
        } catch (const std::exception &e) {
            return makeToken(TokenKind::Unknown, buf, std::monostate{}, line, col);
        }
    }

    Token Lexer::readString() {
        size_t line = src_->line(), col = src_->column();
        std::string buf;
        int start = src_->get();
        (void) start;
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
        return makeToken(TokenKind::String, buf, TokenValue(buf), line, col);
    }

    Token Lexer::makeToken(TokenKind kind, std::string lexeme, TokenValue value, size_t line, size_t column) {
        Token t;
        t.kind = kind;
        t.lexeme = std::move(lexeme);
        t.value = std::move(value);
        t.pos.line = line;
        t.pos.column = column;
        return t;
    }
}
