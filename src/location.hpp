#pragma once

#include <ostream>
#include <string>
#include <variant>

#include "qa_x86.hpp"

namespace as {
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

struct StackLocation {
    int offset;
    int size;
};

using Location =
    std::variant<std::monostate, Temp, HardcodedRegister, StackLocation>;

[[nodiscard]] std::string toAsm(const Location &loc);

[[nodiscard]] int SizeOf(Location loc);

} // namespace as