cmake_minimum_required(VERSION 3.10)
project(Lexer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_library(parser_lib
        include/tools.h
        include/parser.h
        include/ast.h
        src/parser.cpp
        src/lexer.cpp
        include/ast_dump.h
        src/ast_dump.cpp
        src/ast.cpp
        src/source.cpp
        src/interpreter.cpp

)

target_include_directories(parser_lib PUBLIC include)

add_executable(interpreter_demo
        src/main.cpp
        include/tools.h
        include/parser.h
        include/ast.h
        src/parser.cpp
        src/lexer.cpp
        include/ast_dump.h
        src/ast_dump.cpp
        src/ast.cpp
        src/source.cpp
        include/runtime_error.h
        include/interpreter.h
        src/interpreter.cpp
)

target_link_libraries(interpreter_demo PRIVATE parser_lib)

add_executable(parser_tests
        tests/tests.cpp
)

target_link_libraries(parser_tests
        PRIVATE
        parser_lib
        gtest_main
)

add_test(NAME ParserTests COMMAND parser_tests)
