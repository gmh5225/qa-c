#pragma once

#include "assem.hpp"
#include <vector>

namespace codegen {
[[nodiscard]] std::string Generate(const std::vector<as::Frame> &frames);
}