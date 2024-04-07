#include <cassert>
#include <stdexcept>

#include "location.hpp"

namespace qa_ir {
bool operator<(const Temp &lhs, const Temp &rhs) {
    return lhs.id < rhs.id;
}

std::ostream &operator<<(std::ostream &os, const Temp &temp) {
    os << "t" << temp.id;
    return os;
}

bool operator<(const HardcodedRegister &lhs, const HardcodedRegister &rhs) {
    return lhs.reg < rhs.reg;
}

std::ostream &operator<<(std::ostream &os, const HardcodedRegister &reg) {
    os << reg.to_asm();
    return os;
}

[[nodiscard]] std::string HardcodedRegister::to_asm() const {
    return target::to_asm(reg, size);
}

std::ostream &operator<<(std::ostream &os, const Value &v) {
    if (std::holds_alternative<Temp>(v)) {
        os << std::get<Temp>(v);
    } else if (std::holds_alternative<HardcodedRegister>(v)) {
        os << std::get<HardcodedRegister>(v);
    } else if (std::holds_alternative<Variable>(v)) {
        os << std::get<Variable>(v).name;
    } else if (std::holds_alternative<int>(v)) {
        os << std::get<int>(v);
    } else {
        throw std::runtime_error("Unknown value type");
    }
    return os;
}

[[nodiscard]] int SizeOf(Value v) {
    if (std::holds_alternative<Temp>(v)) {
        return std::get<Temp>(v).size;
    } else if (std::holds_alternative<HardcodedRegister>(v)) {
        return std::get<HardcodedRegister>(v).size;
    } else if (std::holds_alternative<Variable>(v)) {
        return std::get<Variable>(v).size;
    } else if (std::holds_alternative<int>(v)) {
        return 4;
    } else {
        throw std::runtime_error("Unknown value type");
    }
}

[[nodiscard]] int SizeOfWhatItPointsTo(Value v) {
    if (std::holds_alternative<Variable>(v)) {
        return std::get<Variable>(v).size;
    }
    throw std::runtime_error("Unknown value type");
}

} // namespace qa_ir