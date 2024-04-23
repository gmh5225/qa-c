#include "../include/assem.hpp"

#include <cassert>
#include <map>
#include <stdexcept>

namespace qa_ir {
[[nodiscard]] std::vector<Frame> Produce_IR(
    const std::vector<std::unique_ptr<ast::AstNode>>& nodes) {
    std::vector<Frame> frames;
    for (const auto& node : nodes) {
        auto ctx = Ctx{.counter = 0,
                       .labelCounter = 0,
                       .variableUsage = {},
                       .variables = {}};
    }
    return frames;
}
}  // namespace qa_ir
