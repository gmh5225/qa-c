#pragma once

#include <vector>

#include "assem.hpp"
#include "operation.hpp"
#include "qa_x86.hpp"

namespace optimizer {
[[nodiscard]] std::vector<target::Frame>
optimize_lowered(const std::vector<target::Frame> &frames);

[[nodiscard]] std::vector<qa_ir::Frame>
optimize(const std::vector<qa_ir::Frame> &frames);
} // namespace optimizer
