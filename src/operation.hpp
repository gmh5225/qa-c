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
    int depth = 1;
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

struct Call {
    std::string name;
    std::vector<Value> args;
    Value dst;
};

std::ostream &operator<<(std::ostream &os, const Label &label);

struct LabelDef {
    Label label;
};

struct DerefStore {
    Value dst;
    Value src;
};

using Operation =
    std::variant<Mov, Ret, Add, Sub, MovR, Addr, Deref, StoreAddr, Compare,
    Equal, ConditionalJump, LabelDef, Call, DerefStore>;

std::ostream &operator<<(std::ostream &os, const Operation &ins);

} // namespace qa_ir