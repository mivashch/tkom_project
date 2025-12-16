#include <iostream>
#include <memory>
#include <string>
#include "source.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "ast_dump.h"

using namespace minilang;

int main(int argc, char **argv) {
        std::unique_ptr<Source> src;
        if (argc > 1) {
            src = makeFileSource(argv[1]);
        } else {
            std::string sample = R"(
fun int add(a: int, b: float) {
    a = 2;
    return a + b;
}

result = add(2, 3);
fun int factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

f = factorial(5); // wynik: 120

fun fun get_func() {
    return inc;
}

fun int inc(x: int) {
    return x + 1;
}

get_func()(7);
)";
            src = makeStringSource(sample);
        }

        Lexer lexer(std::move(src));

        Parser parser(lexer);

        std::unique_ptr<Program> prog = parser.parseProgram();

        if (!prog) {
            std::cerr << "Parse error: "
                    << parser.lastError().value_or("unknown error") << "\n";
            return 1;
        }

        std::cout << "Parsing completed successfully!\n";

        AstPrinter printer(std::cout);
        printer.dump(*prog);




        return 0;
    }
