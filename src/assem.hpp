#pragma once

#include "ast.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace as {

[[nodiscard]] inline std::string registerString(std::string basestring,
        int size) {
    std::cout << "registerString: " << basestring << " " << size << std::endl;
    if (basestring[0] == 'r') {
        if (size == 8) {
            return basestring;
        }
        return basestring + "d";
    }
    if (size == 8) {
        return "r" + basestring;
    }
    if (basestring[0] == 'e') {
        return basestring;
    }
    return "e" + basestring;
}
struct Temp {
    int id;
    int size;
};

struct HardcodedRegister {
    std::string name;
    int size;

    std::string toString() const {
        return registerString(name, size);
    }
};

struct StackLocation {
    int offset;
    int size;
};

inline bool operator<(const as::Temp &lhs, const as::Temp &rhs) {
    return lhs.id < rhs.id;
}
inline bool operator<(const as::HardcodedRegister &lhs,
                      const as::HardcodedRegister &rhs) {
    return lhs.name < rhs.name;
}

using Location =
    std::variant<std::monostate, Temp, HardcodedRegister, StackLocation>;

[[nodiscard]] std::string toAsm(const Location &loc);

[[nodiscard]] int SizeOf(Location loc);

enum class OpCode { Mov, LoadI };

struct Operation {
    OpCode op;
    Location dest;
    Location src;

    std::optional<int> value;

    HardcodedRegister getDestReg() const {
        return std::get<HardcodedRegister>(dest);
    }

    HardcodedRegister getSrcReg() const {
        return std::get<HardcodedRegister>(src);
    }
};

Operation LoadI(Location dst, int value);

inline Operation Mov(Location dst, Location src) {
    return Operation{OpCode::Mov, {dst}, {src}, std::nullopt};
}

struct Label {
    std::string name;
};

using Instruction = std::variant<Operation, Label>;

std::ostream &operator<<(std::ostream &os, const Operation &ins);

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

[[nodiscard]] inline HardcodedRegister HardcodedReg(std::string basestring,
        int size) {
    assert(size == 8 || size == 4);
    return HardcodedRegister{registerString(basestring, size), size};
}

[[nodiscard]] std::vector<Frame>
As_Instructions(const std::vector<std::unique_ptr<ast::Node>> &nodes);
} // namespace as