#pragma once

#include <iostream>
#include <optional>
#include <variant>

#include "location.hpp"

namespace qa_ir {

struct Mov {
    Value dst;
    Value src;
};

struct Ret {
    Value value;
};

struct Add {
    Value dst;

    Value left;
    Value right;
};

struct Sub {
    Value dst;
    Value left;
    Value right;
};

struct MovR {
    Value dst;
    HardcodedRegister src;
};

struct Addr {
    Value dst;
    Value src;
};

struct Deref {
    Value dst;
    Value src;
};

struct StoreAddr {
    Value dst;
    Value src;
};

struct Equal {
    Value dst;
    Value left;
    Value right;
};

using Operation =
    std::variant<Mov, Ret, Add, Sub, MovR, Addr, Deref, StoreAddr, Equal>;

std::ostream &operator<<(std::ostream &os, const Operation &ins);

} // namespace qa_ir