#pragma once

#include "assem.hpp"
#include <vector>

namespace codegen {
[[nodiscard]] std::string Generate(const std::vector<target::Frame> &frames);
}