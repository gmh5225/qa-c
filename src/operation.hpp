#pragma once

#include <iostream>
#include <optional>
#include <variant>

#include "location.hpp"

namespace qa_ir {

struct Label {
    std::string name;
};

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

struct Compare {
    Value left;
    Value right;
};

struct Equal {
    Value dst;
    Value left;
    Value right;
};

struct ConditionalJump {
    Label trueLabel;
    Label falseLabel;
};

std::ostream &operator<<(std::ostream &os, const Label &label);

struct LabelDef {
    Label label;
};

using Operation = std::variant<Mov, Ret, Add, Sub, MovR, Addr, Deref, StoreAddr,
      Compare, Equal, ConditionalJump, LabelDef>;

std::ostream &operator<<(std::ostream &os, const Operation &ins);

} // namespace qa_ir