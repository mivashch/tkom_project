#pragma once
#include <string>

struct Position {
        size_t line = 1;
        size_t column = 1;
    };

inline bool isIdentifierStart(char c) { return std::isalpha((unsigned char) c) || c == '_'; }
inline bool isIdentifierPart(char c)  { return std::isalnum((unsigned char) c) || c == '_'; }
