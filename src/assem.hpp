#pragma once

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "ast.hpp"
#include "location.hpp"
#include "operation.hpp"
#include "qa_x86.hpp"

namespace qa_ir {

struct Frame {
    std::string name;
    std::vector<Operation> instructions;
    int size = 0;
};

[[nodiscard]] std::vector<Frame>
Produce_IR(const std::vector<std::unique_ptr<ast::Node>> &nodes);

} // namespace qa_ir