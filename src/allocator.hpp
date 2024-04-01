#pragma once

#include "assem.hpp"
#include <vector>

namespace allocator {
[[nodiscard]] std::vector<target::Frame>
rewrite(const std::vector<target::Frame> &frames);
}