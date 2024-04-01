#include <concepts>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <variant>
#include <vector>

#include "allocator.hpp"
#include "qa_x86.hpp"

namespace allocator {

struct Ctx {
public:
  std::set<int> usedRegs = {};
  std::map<target::Register, target::BaseRegister> mapping = {};

  target::BaseRegister getReg() {
    int i = 0;
    for (const auto &reg : target::general_regs) {
      if (usedRegs.find(i) == usedRegs.end()) {
        usedRegs.insert(i);
        return reg;
      }
      i++;
    }
    throw std::runtime_error("No free registers");
  }

  void freeReg(target::BaseRegister freedReg) {
    for (unsigned long i = 0; i < target::general_regs.size(); i++) {
      if (target::general_regs[i] == freedReg) {
        usedRegs.erase(i);
        return;
      }
    }
    throw std::runtime_error("Register not found");
  }
};

struct FirstLastUse {
  std::map<int, int> firstUse = {};
  std::map<int, int> lastUse = {};
};

template <target::HasRegisterSrc T>
std::optional<target::VirtualRegister> get_src_register(const T &instruction) {
  if (auto reg = std::get_if<target::VirtualRegister>(&instruction.src)) {
    return reg;
  }
  return std::nullopt;
}

template <target::HasRegisterDest T>
std::optional<target::VirtualRegister> get_dest_register(const T &instruction) {
  if (auto reg = std::get_if<target::VirtualRegister>(&instruction.dst)) {
    return reg;
  }
  return std::nullopt;
}

template <target::HasRegisterSrc T>
std::optional<int> get_src_register_virtual_id(const T &instruction) {
  if (auto reg = std::get_if<target::VirtualRegister>(&instruction.src)) {
    return reg->id; // Assuming VirtualRegister has an `id` member
  }
  return std::nullopt;
}

template <target::HasRegisterDest T>
std::optional<int> get_dest_register_virtual_id(const T &instruction) {
  if (auto reg = std::get_if<target::VirtualRegister>(&instruction.dst)) {
    return reg->id; // Assuming VirtualRegister has an `id` member
  }
  return std::nullopt;
}

std::optional<int>
get_src_virtual_id_if_present(const target::Instruction &instruction) {
  return std::visit(
      [](const auto &ins) -> std::optional<int> {
        if constexpr (target::HasRegisterSrc<decltype(ins)>) {
          return get_register_virtual_id(ins);
        } else {
          return std::nullopt;
        }
      },
      instruction);
}

std::optional<int>
get_dest_virtual_id_if_present(const target::Instruction &instruction) {
  return std::visit(
      [](const auto &ins) -> std::optional<int> {
        if constexpr (target::HasRegisterDest<decltype(ins)>) {
          return get_register_virtual_id(ins);
        } else {
          return std::nullopt;
        }
      },
      instruction);
}

void set_src_register_if_present(target::Instruction &instruction,
                                 target::Register reg) {
  std::visit(
      [reg](const auto &ins) -> void {
        if constexpr (target::HasRegisterSrc<decltype(ins)>) {
          ins.src = reg;
        }
      },
      instruction);
}

void set_dest_register_if_present(target::Instruction &instruction,
                                  target::Register reg) {
  std::visit(
      [reg](const auto &ins) -> void {
        if constexpr (target::HasRegisterDest<decltype(ins)>) {
          ins.dst = reg;
        }
      },
      instruction);
}

std::optional<target::VirtualRegister>
get_src_register_if_present(target::Instruction &instruction) {
  return std::visit(
      [](const auto &ins) -> std::optional<target::VirtualRegister> {
        if constexpr (target::HasRegisterSrc<decltype(ins)>) {
          return get_src_register(ins);
        }
        return std::nullopt;
      },
      instruction);
}

std::optional<target::VirtualRegister>
get_dest_register_if_present(target::Instruction &instruction) {
  return std::visit(
      [](const auto &ins) -> std::optional<target::VirtualRegister> {
        if constexpr (target::HasRegisterDest<decltype(ins)>) {
          return get_dest_register(ins);
        }
        return std::nullopt;
      },
      instruction);
}

[[nodiscard]] FirstLastUse getFirstUse(const target::Frame &frame) {
  FirstLastUse fl = {};
  int i = 0;
  for (const auto &ins : frame.instructions) {
    ++i;
    const auto operation = ins;
    std::optional<int> srcId = get_src_virtual_id_if_present(operation);
    std::optional<int> dstId = get_dest_virtual_id_if_present(operation);
    for (auto id : {srcId, dstId}) {
      if (id.has_value()) {
        if (fl.firstUse.find(id.value()) == fl.firstUse.end()) {
          fl.firstUse[id.value()] = i;
        }
        fl.lastUse[id.value()] = i;
      }
    }
  }
  return fl;
}

[[nodiscard]] std::map<target::VirtualRegister, target::VirtualRegister>
remap(target::Frame &frame) {
  std::map<target::VirtualRegister, target::VirtualRegister> remappedRegisters =
      {};
  std::map<target::VirtualRegister, int> newFirstUsed = {};
  int idx = 0;
  for (const auto &i : frame.instructions) {
    idx += 1;
    auto operation = i;
    std::optional<target::VirtualRegister> src;
    const auto register_src = target::register_src(operation);
    if (register_src.has_value() &&
        std::holds_alternative<target::VirtualRegister>(*register_src)) {
      src = std::get<target::VirtualRegister>(*register_src);
      if (newFirstUsed.find(*src) == newFirstUsed.end()) {
        newFirstUsed[*src] = idx;
      }
    }
    const auto register_dest = target::register_dest(operation);
    if (register_dest.has_value() &&
        std::holds_alternative<target::VirtualRegister>(*register_dest)) {
      const auto dest = std::get<target::VirtualRegister>(*register_dest);
      if (std::holds_alternative<target::Mov>(operation) == false) {
        continue;
      }
      // if not found in newFirstused, (ie first used as a dest) then we need to
      // remap
      if (newFirstUsed.find(dest) == newFirstUsed.end()) {
        if (src.has_value()) {
          // if src is in remapped registers, then we need to remap it
          if (remappedRegisters.find(*src) != remappedRegisters.end()) {
            remappedRegisters[dest] = remappedRegisters[*src];
          } else {
            remappedRegisters[dest] = *src;
          }
        }
      }
    }
  }
  return remappedRegisters;
}

[[nodiscard]] target::Frame rewrite(const target::Frame &frame, Ctx &ctx) {
  target::Frame newFrame = frame;
  const auto fl = getFirstUse(frame);
  auto firstUse = fl.firstUse;
  auto lastUse = fl.lastUse;
  auto remappedRegisters = remap(newFrame);
  for (const auto &entry : remappedRegisters) {
    const auto prev = entry.first;
    const auto newReg = entry.second;
    firstUse[newReg.id] = std::min(firstUse[newReg.id], firstUse[prev.id]);
    lastUse[newReg.id] = std::max(lastUse[newReg.id], lastUse[prev.id]);
  }
  std::vector<target::Instruction> newInstructions = {};
  int idx = 0;
  for (auto &i : newFrame.instructions) {
    idx += 1;
    auto operation = i;
    auto src_op = get_src_register_if_present(operation);
    if (src_op.has_value()) {
      const auto src = src_op.value();
      if (lastUse.find(src.id) == lastUse.end()) {
        throw std::runtime_error("Last use not found");
      }
      if (remappedRegisters.find(src) != remappedRegisters.end()) {
        const auto remapped = remappedRegisters.at(src);
        set_src_register_if_present(operation, remapped);
      }
      if (firstUse[src.id] == idx) {
        ctx.mapping[src] = ctx.getReg();
      }
      if (ctx.mapping.find(src) == ctx.mapping.end()) {
        ctx.mapping[src] = ctx.getReg();
      }
      set_src_register_if_present(
          operation, target::HardcodedRegister{ctx.mapping.at(src), src.size});
      if (lastUse[src.id] == idx) {
        ctx.freeReg(ctx.mapping.at(src));
      }
    }
    auto dest_op = get_dest_register_if_present(operation);
    if (dest_op.has_value()) {
      const auto dest = dest_op.value();
      if (remappedRegisters.find(dest) != remappedRegisters.end()) {
        const auto remapped = remappedRegisters.at(dest);
        set_dest_register_if_present(operation, remapped);
      }
      if (firstUse[dest.id] == idx) {
        ctx.mapping[dest] = ctx.getReg();
      }
      if (ctx.mapping.find(dest) == ctx.mapping.end()) {
        ctx.mapping[dest] = ctx.getReg();
      }
      set_dest_register_if_present(
          operation,
          target::HardcodedRegister{ctx.mapping.at(dest), dest.size});
      if (lastUse[dest.id] == idx) {
        ctx.freeReg(ctx.mapping.at(dest));
      }
    }
    newInstructions.push_back(operation);
  }
  newFrame.instructions = newInstructions;
  return newFrame;
}

[[nodiscard]] std::vector<target::Frame>
rewrite(const std::vector<target::Frame> &frames) {
  std::vector<target::Frame> newFrames;
  for (const auto &frame : frames) {
    Ctx ctx = Ctx{};
    newFrames.push_back(rewrite(frame, ctx));
  }
  return newFrames;
}
} // namespace allocator
