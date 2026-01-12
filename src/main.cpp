#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace minilang;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: interpreter <file>\n";
        return 1;
    }

    std::ifstream in(argv[1]);
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
        std::cerr << e.what() << "\n";
    }
    catch (const RuntimeError& e) {
        std::cerr << e.what() << "\n";
    }
}
