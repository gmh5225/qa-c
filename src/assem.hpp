#pragma once

#include "ast.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "location.hpp"
#include "operation.hpp"
#include "qa_x86.hpp"

namespace as {

struct Label {
    std::string name;
};

using Instruction = std::variant<Operation, Label>;

inline std::ostream &operator<<(std::ostream &os, const Instruction &ins) {
    std::visit([&os](const auto &arg) {
        os << arg;
    }, ins);
    return os;
}

struct Frame {
    std::string name;
    std::vector<Instruction> instructions;
    int size = 0;
};

[[nodiscard]] std::vector<Frame>
As_Instructions(const std::vector<std::unique_ptr<ast::Node>> &nodes);

} // namespace as