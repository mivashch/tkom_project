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
const y = 2;
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
