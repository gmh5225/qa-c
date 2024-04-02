#pragma once

#include <ostream>
#include <string>
#include <variant>

#include "qa_x86.hpp"

namespace qa_ir {
struct Temp {
    int id;
    int size;
};

bool operator<(const Temp &lhs, const Temp &rhs);
std::ostream &operator<<(std::ostream &os, const Temp &temp);

struct HardcodedRegister {
    target::BaseRegister reg;
    int size;

    [[nodiscard]] std::string to_asm() const;
};
bool operator<(const HardcodedRegister &lhs, const HardcodedRegister &rhs);
std::ostream &operator<<(std::ostream &os, const HardcodedRegister &reg);

struct Variable {
    std::string name;
    int version = 0;
    int size = 0;
};

using Value =
    std::variant<std::monostate, Temp, HardcodedRegister, Variable, int>;

std::ostream &operator<<(std::ostream &os, const Value &v);

[[nodiscard]] int SizeOf(Value v);

} // namespace qa_ir