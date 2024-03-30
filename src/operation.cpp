#include <stdexcept>

#include "operation.hpp"

namespace as {

HardcodedRegister Operation::getDestReg() const {
    return std::get<HardcodedRegister>(dest);
}

HardcodedRegister Operation::getSrcReg() const {
    return std::get<HardcodedRegister>(src);
}

Operation LoadI(Location dst, int value) {
    return Operation{.op = OpCode::LoadI,
                     .dest = dst,
                     .src = std::monostate{},
                     .value = value};
}

Operation StoreI(Location dst, int value) {
    return Operation{.op = OpCode::StoreI,
                     .dest = dst,
                     .src = std::monostate{},
                     .value = value};
}

Operation Load(Location dst, Location src) {
    return Operation{.op = OpCode::Load, .dest = dst, .src = src};
}

Operation Store(Location dst, Location src) {
    return Operation{.op = OpCode::Store, .dest = dst, .src = src};
}

Operation Mov(Location dst, Location src) {
    return Operation{OpCode::Mov, {dst}, {src}, std::nullopt};
}

Operation Deref(Location dst, Location src) {
    return Operation{OpCode::Deref, {dst}, {src}, std::nullopt};
}

Operation Addr(Location dst, Location src) {
    return Operation{OpCode::Addr, {dst}, {src}, std::nullopt};
}

std::ostream &operator<<(std::ostream &os, const Operation &ins) {
    switch (ins.op) {
    case OpCode::Mov:
        os << "mov ";
        break;
    case OpCode::LoadI:
        os << "loadI ";
        break;
    case OpCode::Deref:
        os << "deref ";
        break;
    case OpCode::Addr:
        os << "addr ";
        break;
    case OpCode::StoreI:
        os << "storeI ";
        break;
    case OpCode::Load:
        os << "load ";
        break;
    case OpCode::Store:
        os << "store ";
        break;
    default:
        throw std::runtime_error("Unknown OpCode");
    }
    if (auto dst = std::get_if<Temp>(&ins.dest)) {
        os << *dst << " ";
    } else if (auto dst = std::get_if<HardcodedRegister>(&ins.dest)) {
        os << *dst << " ";
    } else if (auto dst = std::get_if<StackLocation>(&ins.dest)) {
        os << "rbp - " << dst->offset << " ";
    }
    if (auto src = std::get_if<Temp>(&ins.src)) {
        os << *src << " ";
    } else if (auto src = std::get_if<HardcodedRegister>(&ins.src)) {
        os << *src << " ";
    } else if (auto src = std::get_if<StackLocation>(&ins.src)) {
        os << "rbp - " << src->offset << " ";
    }
    if (ins.value.has_value()) {
        os << "value: " << ins.value.value();
    }
    return os;
}

} // namespace as