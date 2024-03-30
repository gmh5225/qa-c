#include "qa_x86.hpp"

namespace target {
std::ostream &operator<<(std::ostream &os, BaseRegister reg) {
    switch (reg) {
    case BaseRegister::AX:
        os << "ax";
        break;
    case BaseRegister::BX:
        os << "bx";
        break;
    case BaseRegister::CX:
        os << "cx";
        break;
    case BaseRegister::DX:
        os << "dx";
        break;
    case BaseRegister::SI:
        os << "si";
        break;
    case BaseRegister::DI:
        os << "di";
        break;
    case BaseRegister::R8:
        os << "r8";
        break;
    case BaseRegister::R9:
        os << "r9";
        break;
    case BaseRegister::R10:
        os << "r10";
        break;
    case BaseRegister::R11:
        os << "r11";
        break;
    case BaseRegister::R12:
        os << "r12";
        break;
    case BaseRegister::R13:
        os << "r13";
        break;
    case BaseRegister::R14:
        os << "r14";
        break;
    case BaseRegister::R15:
        os << "r15";
        break;
    }
    return os;
}

[[nodiscard]] std::string to_asm(BaseRegister reg, int size) {
    switch (reg) {
    case BaseRegister::AX:
        return size == 4 ? "eax" : "rax";
    case BaseRegister::BX:
        return size == 4 ? "ebx" : "rbx";
    case BaseRegister::CX:
        return size == 4 ? "ecx" : "rcx";
    case BaseRegister::DX:
        return size == 4 ? "edx" : "rdx";
    case BaseRegister::SI:
        return "rsi";
    case BaseRegister::DI:
        return "rdi";
    case BaseRegister::R8:
        return "r8";
    case BaseRegister::R9:
        return "r9";
    case BaseRegister::R10:
        return size == 4 ? "r10d" : "r10";
    case BaseRegister::R11:
        return size == 4 ? "r11d" : "r11";
    case BaseRegister::R12:
        return size == 4 ? "r12d" : "r12";
    case BaseRegister::R13:
        return size == 4 ? "r13d" : "r13";
    case BaseRegister::R14:
        return size == 4 ? "r14d" : "r14";
    case BaseRegister::R15:
        return size == 4 ? "r15d" : "r15";
    default:
        throw std::runtime_error("to_asm not implemented");
    }
}
} // namespace target