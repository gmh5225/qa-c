#pragma once

#include <iostream>
#include <optional>
#include <variant>

#include "location.hpp"

namespace as {
enum class OpCode { Mov, Load, LoadI, Deref, Addr, StoreI, Store };

struct Operation {
    OpCode op;
    Location dest;
    Location src;

    std::optional<int> value;

    HardcodedRegister getDestReg() const;

    HardcodedRegister getSrcReg() const;
};

Operation LoadI(Location dst, int value);
Operation StoreI(Location dst, int value);
Operation Load(Location dst, Location src);
Operation Store(Location dst, Location src);
Operation Mov(Location dst, Location src);
Operation Deref(Location dst, Location src);
Operation Addr(Location dst, Location src);

std::ostream &operator<<(std::ostream &os, const Operation &ins);

} // namespace as