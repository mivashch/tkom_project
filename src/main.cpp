#include "lexer.h"
#include "source.h"
#include "token.h"
#include <iostream>
#include <iomanip>

using namespace minilang;


static void printToken(const Token &t) {
    std::cout << "[" << t.pos.line << ":" << t.pos.column << "] ";
    switch (t.kind) {
        case TokenKind::EndOfFile: std::cout << "<EOF>";
            break;
        case TokenKind::Identifier: std::cout << "IDENT(" << t.lexeme << ")";
            break;
        case TokenKind::NumberInt: std::cout << "INT(" << std::get<long long>(t.value) << ")";
            break;
        case TokenKind::NumberFloat: std::cout << "FLOAT(" << std::get<double>(t.value) << ")";
            break;
        case TokenKind::String: std::cout << "STR(\"" << std::get<std::string>(t.value) << "\")";
            break;
        case TokenKind::Bool: std::cout << "BOOL(" << (std::get<bool>(t.value) ? "true" : "false") << ")";
            break;
        case TokenKind::Keyword: std::cout << "KW(" << t.lexeme << ")";
            break;
        case TokenKind::Operator: std::cout << "OP(" << t.lexeme << ")";
            break;
        case TokenKind::Punctuator: std::cout << "PUNC(" << t.lexeme << ")";
            break;
        default: std::cout << "UNK(" << t.lexeme << ")";
            break;
    }
    std::cout << std::endl;
}


int main(int argc, char **argv) {
    try {
        std::unique_ptr<Source> src;
        if (argc > 1) src = makeFileSource(argv[1]);
        else {
            // przyklad z dokumentacji
            std::string sample = R"(
fun int add(int a, int b) {
    return a + b;
}

result = add(2, 3);
fun int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

f = factorial(5); // wynik: 120

fun fun get_func() {
    return inc;
}

fun int inc(int x) {
    return x + 1;
}

get_func()(7);)";
            src = makeStringSource(sample);
        }
        Lexer lex(std::move(src));
        while (true) {
            Token t = lex.nextToken();
            printToken(t);
            if (t.kind == TokenKind::EndOfFile) break;
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }
    return 0;
}
