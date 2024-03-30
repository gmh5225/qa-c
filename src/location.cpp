#include <cassert>
#include <stdexcept>

#include "location.hpp"

namespace as {
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

[[nodiscard]] std::string toAsm(const Location &loc) {
    if (std::holds_alternative<Temp>(loc)) {
        throw std::runtime_error("Temp not implemented");
    } else if (std::holds_alternative<HardcodedRegister>(loc)) {
        return std::get<HardcodedRegister>(loc).to_asm();
    } else if (std::holds_alternative<StackLocation>(loc)) {
        auto size = std::get<StackLocation>(loc).size;
        if (size == 4) {
            return "dword [rbp - " +
                   std::to_string(std::get<StackLocation>(loc).offset) + "]";
        } else if (size == 8) {
            return "qword [rbp - " +
                   std::to_string(std::get<StackLocation>(loc).offset) + "]";
        }
    }
    throw std::runtime_error("toAsm not implemented");
}

[[nodiscard]] int SizeOf(const Location loc) {
    if (std::holds_alternative<Temp>(loc)) {
        auto res = std::get<Temp>(loc).size;
        return res;
    } else if (std::holds_alternative<HardcodedRegister>(loc)) {
        auto res = std::get<HardcodedRegister>(loc).size;
        return res;
    } else {
        auto res = std::get<StackLocation>(loc).size;
        return res;
    }
    throw std::runtime_error("SizeOf not implemented");
}

} // namespace as