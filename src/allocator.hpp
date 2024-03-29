#pragma once

#include "assem.hpp"
#include <vector>

namespace allocator {
[[nodiscard]] std::vector<as::Frame>
rewrite(const std::vector<as::Frame> &frames);
}