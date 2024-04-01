#pragma once

#include <concepts>
#include <iostream>
#include <optional>
#include <string>
#include <variant>
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
    BaseRegister::CX, BaseRegister::R8, BaseRegister::R9};
// general purpose registers
// these are disjoint from the param_regs, so that calls don't clobber them
inline const std::vector<BaseRegister> general_regs = {
    BaseRegister::AX,  BaseRegister::BX,  BaseRegister::R10, BaseRegister::R11,
    BaseRegister::R12, BaseRegister::R13, BaseRegister::R14, BaseRegister::R15};

[[nodiscard]] std::string to_asm(BaseRegister reg, int size);

struct HardcodedRegister {
  BaseRegister reg;
  int size;
};

struct StackLocation {
  int offset;
};

struct VirtualRegister {
  int id;
  int size;
};
using Register = std::variant<HardcodedRegister, VirtualRegister>;
// comparison operators for registers
bool operator<(const Register &lhs, const Register &rhs);

using Location = std::variant<Register, StackLocation>;

struct Mov {
  Register dst;
  Register src;

  HardcodedRegister src_reg() const { return std::get<HardcodedRegister>(src); }
  HardcodedRegister dst_reg() const { return std::get<HardcodedRegister>(dst); }
};

struct LoadI {
  Register dst;
  int value;
};

struct Load {
  Register dst;
  StackLocation src;
};

struct StoreI {
  int offset;
  int value;
};

struct Store {
  StackLocation dst;
  Register src;
};

struct Jump {
  std::string label;
};

using Instruction = std::variant<Mov, LoadI, StoreI, Store, Load, Jump>;

template <typename T>
concept HasRegisterSrc = requires(T t) {
  { t.src } -> std::same_as<target::Register>;
};

template <typename T>
concept HasRegisterDest = requires(T t) {
  { t.dst } -> std::same_as<target::Register>;
};

std::optional<Register> register_src(const Instruction &ins);
std::optional<Register> register_dest(const Instruction &ins);

struct Frame {
  std::string name;
  std::vector<Instruction> instructions;
  int size = 0;
};

} // namespace target