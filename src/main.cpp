#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

using namespace minilang;

static void runFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "Cannot open file: " << filename << "\n";
        return;
    }

    std::stringstream buffer;
    buffer << in.rdbuf();

    try {
        Lexer lx(makeStringSource(buffer.str()));
        Parser p(lx);
        auto program = p.parseProgram();

        Interpreter interp;
        interp.execute(*program);
    }
    catch (const ParseError& e) {
        std::cerr << "ParseError: " << e.what() << "\n";
    }
    catch (const RuntimeError& e) {
        std::cerr << "RuntimeError: " << e.what() << "\n";
    }
}

static void runRepl() {
    Interpreter interp;

    std::cout << "Kartka interactive interpreter\n";
    std::cout << "Type :quit or Ctrl+D to exit\n";

    while (true) {
        std::cout << "> ";
        std::string line;

        if (!std::getline(std::cin, line))
            break;

        if (line == ":quit" || line == ":q")
            break;

        try {
            Lexer lx(makeStringSource(line));
            Parser p(lx);
            auto program = p.parseProgram();

            program->accept(interp);

            Value& v = interp.getLastValue();
            if (!std::holds_alternative<std::monostate>(v)) {
                std::visit(OutputValue{std::cout}, v);
                std::cout << "\n";
            }
        }
        catch (const ParseError& e) {
            std::cout << "ParseError: " << e.what() << std::endl;

        }
        catch (const RuntimeError& e) {
            std::cout << "RuntimeError: " << e.what() << std::endl;

        }
        catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
}

int main(int argc, char** argv) {

    if (argc == 1) {
        runRepl();
        return 0;
    }

    if (argc == 2) {
        runFile(argv[1]);
        return 0;
    }

    std::cerr << "Usage:\n";
    std::cerr << "  interpreter <file>   # run file\n";
    std::cerr << "  interpreter          # interactive REPL\n";
    return 1;
}
