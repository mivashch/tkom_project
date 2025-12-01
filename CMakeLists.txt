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

add_library(lexer_lib
        src/source.cpp
        src/lexer.cpp
)

target_include_directories(lexer_lib PUBLIC include)

add_executable(lexer_demo
        src/main.cpp
)

target_link_libraries(lexer_demo PRIVATE lexer_lib)

add_executable(lexer_tests
        tests/tests.cpp
)

target_link_libraries(lexer_tests
        PRIVATE
        lexer_lib
        gtest_main
)

add_test(NAME LexerTests COMMAND lexer_tests)
