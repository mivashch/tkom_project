#pragma once
#include <stdexcept>
#include "tools.h"

namespace minilang {

    struct RuntimeError : std::runtime_error {
        Position pos;
        RuntimeError(Position p, const std::string& msg)
            : std::runtime_error(msg), pos(p) {}
    };

}
