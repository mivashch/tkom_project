#include "lexer.h"
#include "source.h"
#include "token.h"
#include <iostream>
#include <iomanip>

using namespace minilang;

static const char *tokenKindToString(TokenKind k) {
    switch (k) {
        case TokenKind::EndOfFile: return "EOF";

        case TokenKind::Identifier: return "IDENT";
        case TokenKind::NumberInt: return "INT";
        case TokenKind::NumberFloat: return "FLOAT";
        case TokenKind::String: return "STRING";
        case TokenKind::Bool: return "BOOL";
        case TokenKind::Keyword: return "KW";

        case TokenKind::OpAnd: return "AND";
        case TokenKind::OpRefStarRef: return "REF_STAR_REF";
        case TokenKind::OpAssign: return "ASSIGN";
        case TokenKind::OpEq: return "EQ";
        case TokenKind::OpArrow: return "ARROW";
        case TokenKind::OpDoubleArrow: return "DOUBLE_ARROW";
        case TokenKind::OpOr: return "OR";
        case TokenKind::OpNot: return "NOT";
        case TokenKind::OpNotEq: return "NEQ";
        case TokenKind::OpLess: return "LT";
        case TokenKind::OpLessEq: return "LE";
        case TokenKind::OpGreater: return "GT";
        case TokenKind::OpGreaterEq: return "GE";
        case TokenKind::OpPlus: return "PLUS";
        case TokenKind::OpMinus: return "MINUS";
        case TokenKind::OpMul: return "MUL";
        case TokenKind::OpDiv: return "DIV";
        case TokenKind::OpMod: return "MOD";

        case TokenKind::Punctuator: return "PUNC";
        case TokenKind::Comment: return "COMMENT";
        case TokenKind::Unknown: return "UNKNOWN";
    }
    return "???";
}

static void printToken(const Token &t) {
    std::cout << "[" << t.pos.line << ":" << t.pos.column << "] ";

    TokenKind k = t.getKind();
    std::cout << tokenKindToString(k);

    switch (k) {
        case TokenKind::Identifier:
        case TokenKind::Keyword:
        case TokenKind::Punctuator:
        case TokenKind::OpAnd:
        case TokenKind::OpRefStarRef:
        case TokenKind::OpAssign:
        case TokenKind::OpEq:
        case TokenKind::OpArrow:
        case TokenKind::OpDoubleArrow:
        case TokenKind::OpOr:
        case TokenKind::OpNot:
        case TokenKind::OpNotEq:
        case TokenKind::OpLess:
        case TokenKind::OpLessEq:
        case TokenKind::OpGreater:
        case TokenKind::OpGreaterEq:
        case TokenKind::OpPlus:
        case TokenKind::OpMinus:
        case TokenKind::OpMul:
        case TokenKind::OpDiv:
        case TokenKind::OpMod:
        case TokenKind::Unknown:
            std::cout << "(" << t.lexeme << ")";
            break;

        case TokenKind::NumberInt:
            std::cout << "(" << std::get<long long>(t.getValue()) << ")";
            break;

        case TokenKind::NumberFloat:
            std::cout << "(" << std::get<double>(t.getValue()) << ")";
            break;

        case TokenKind::String:
            std::cout << "(\"" << std::get<std::string>(t.getValue()) << "\")";
            break;

        case TokenKind::Bool:
            std::cout << "(" << (std::get<bool>(t.getValue()) ? "true" : "false") << ")";
            break;

        case TokenKind::EndOfFile:
        case TokenKind::Comment:
            break;
    }

    std::cout << std::endl;
}

int main(int argc, char **argv) {
    try {
        std::unique_ptr<Source> src;
        if (argc > 1) {
            src = makeFileSource(argv[1]);
        } else {
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

get_func()(7);
)";
            src = makeStringSource(sample);
        }

        Lexer lex(std::move(src));

        while (true) {
            Token t = lex.nextToken();
            printToken(t);
            if (t.getKind() == TokenKind::EndOfFile)
                break;
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }

    return 0;
}
