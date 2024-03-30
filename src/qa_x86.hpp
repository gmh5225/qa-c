#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace target {

// all registers used.
// x86-64 registers
enum class BaseRegister {
    AX,
    BX,
    CX,
    DX,
    SI,
    DI,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15
};

std::ostream &operator<<(std::ostream &os, BaseRegister reg);

// six system V calling convention registers
inline const std::vector<BaseRegister> param_regs = {
    BaseRegister::DI, BaseRegister::SI, BaseRegister::DX,
    BaseRegister::CX, BaseRegister::R8, BaseRegister::R9
};
// general purpose registers
// these are disjoint from the param_regs, so that calls don't clobber them
inline const std::vector<BaseRegister> general_regs = {
    BaseRegister::AX,  BaseRegister::BX,  BaseRegister::R10, BaseRegister::R11,
    BaseRegister::R12, BaseRegister::R13, BaseRegister::R14, BaseRegister::R15
};

[[nodiscard]] std::string to_asm(BaseRegister reg, int size);

} // namespace target