#include "lower_ir.hpp"
#include <map>
#include <stdexcept>
#include <variant>

namespace target {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

[[nodiscard]] std::vector<Instruction> _Value_To_Location(Register r_dst,
                                                          int v_src, Ctx *ctx) {
  return {LoadI{.dst = r_dst, .value = v_src}};
}

[[nodiscard]] std::vector<Instruction> _Value_To_Location(StackLocation l_dest,
                                                          int v_src, Ctx *ctx) {
  return {StoreI{.offset = l_dest.offset, .value = v_src}};
}

[[nodiscard]] std::vector<Instruction>
_Value_To_Location(Register l, std::monostate m, Ctx *ctx) {
  throw std::runtime_error("Cannot convert monostate to location");
}

[[nodiscard]] std::vector<Instruction>
_Value_To_Location(StackLocation l, std::monostate m, Ctx *ctx) {
  throw std::runtime_error("Cannot convert monostate to location");
}

[[nodiscard]] std::vector<Instruction>
_Value_To_Location(StackLocation l_dest, qa_ir::Temp t_src, Ctx *ctx) {
  const auto reg = ctx->AllocateNewForTemp(t_src);
  return {Store{.dst = l_dest, .src = reg}};
}

[[nodiscard]] std::vector<Instruction>
_Value_To_Location(Register r, qa_ir::Temp t, Ctx *ctx) {
  throw std::runtime_error("Cannot convert temp to register"); // just for now
}

[[nodiscard]] std::vector<Instruction>
_Value_To_Location(Register r, qa_ir::HardcodedRegister t, Ctx *ctx) {
  throw std::runtime_error(
      "Cannot convert hardcoded register to register"); // just for now
}

[[nodiscard]] std::vector<Instruction>
_Value_To_Location(Register r_dst, qa_ir::Variable v_src, Ctx *ctx) {
  const auto variableOffset = ctx->variable_offset[v_src.name];
  return {Load{.dst = r_dst, .src = StackLocation{.offset = variableOffset}}};
}

[[nodiscard]] std::vector<Instruction>
_Value_To_Location(StackLocation s_dst, qa_ir::HardcodedRegister r_src,
                   Ctx *ctx) {
  const auto reg = HardcodedRegister{.reg = r_src.reg, .size = r_src.size};
  return {Store{.dst = s_dst, .src = reg}};
}

[[nodiscard]] std::vector<Instruction>
_Value_To_Location(StackLocation s_dst, qa_ir::Variable v_src, Ctx *ctx) {
  const auto reg = ctx->NewRegister(v_src.size);
  auto result = std::vector<Instruction>{};
  // move variable to register
  const auto variableOffset = ctx->variable_offset[v_src.name];
  result.push_back(
      Load{.dst = reg, .src = StackLocation{.offset = variableOffset}});
  // store register to stack
  result.push_back(Store{.dst = s_dst, .src = reg});
  return result;
}

#pragma clang diagnostic pop

Location Ctx::AllocateNew(qa_ir::Value v) {
  if (auto tmp = std::get_if<qa_ir::Temp>(&v)) {
    return VirtualRegister{.id = tempCounter++, .size = tmp->size};
  }
  if (auto variable = std::get_if<qa_ir::Variable>(&v)) {
    const auto variableName = variable->name;
    if (auto it = variable_offset.find(variableName);
        it == variable_offset.end()) {
      stackOffset += variable->size;
      variable_offset[variableName] = stackOffset;
    }
    return StackLocation{.offset = variable_offset.at(variableName)};
  }
  if (auto hardcoded = std::get_if<qa_ir::HardcodedRegister>(&v)) {
    return HardcodedRegister{.reg = hardcoded->reg, .size = hardcoded->size};
  }
  throw std::runtime_error("Cannot allocate new location for value");
}

Register Ctx::AllocateNewForTemp(qa_ir::Temp t) {
  if (auto it = temp_register_mapping.find(t.id);
      it != temp_register_mapping.end()) {
    return it->second;
  }
  const auto reg = VirtualRegister{.id = tempCounter++, .size = t.size};
  temp_register_mapping[t.id] = reg;
  return reg;
}

VirtualRegister Ctx::NewRegister(int size) {
  return VirtualRegister{.id = tempCounter++, .size = size};
}

std::vector<Instruction> Ctx::toLocation(Location l, qa_ir::Value v) {
  return std::visit(
      [this](auto &&arg1, auto &&arg2) {
        return _Value_To_Location(arg1, arg2, this);
      },
      l, v);
}

[[nodiscard]] std::vector<Instruction> LowerMoveInstruction(qa_ir::Mov move,
                                                            Ctx &ctx) {
  auto dest = move.dst;
  auto src = move.src;
  auto destLocation = ctx.AllocateNew(dest);
  return ctx.toLocation(destLocation, src);
}

[[nodiscard]] std::vector<Instruction> LowerReturnInstruction(qa_ir::Ret ret,
                                                              Ctx &ctx) {
  const auto returnValue = ret.value;
  const auto returnValueSize = qa_ir::SizeOf(returnValue);
  const auto returnRegister = HardcodedRegister{.reg = target::BaseRegister::AX,
                                                .size = returnValueSize};
  auto result = ctx.toLocation(returnRegister, returnValue);
  auto jumpInstruction = Jump{.label = "end"};
  result.push_back(jumpInstruction);
  return result;
}

[[nodiscard]] std::vector<Instruction>
LowerInstruction(const qa_ir::Operation &op, Ctx &ctx) {
  return std::visit(
      [&](auto &&arg) -> std::vector<Instruction> {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, qa_ir::Mov>) {
          return LowerMoveInstruction(arg, ctx);
        }
        if constexpr (std::is_same_v<T, qa_ir::Ret>) {
          return LowerReturnInstruction(arg, ctx);
        }

        else {
          throw std::runtime_error(
              "LowerInstruction not implemented for type: " +
              std::to_string(static_cast<int>(op.index())));
        }
      },
      op);
}

[[nodiscard]] std::vector<Frame>
LowerIR(const std::vector<qa_ir::Frame> &frames) {
  std::vector<Frame> result;
  for (const auto &f : frames) {
    std::vector<Instruction> instructions;
    Ctx ctx = Ctx{};
    for (const auto &op : f.instructions) {
      auto ins = LowerInstruction(op, ctx);
      instructions.insert(instructions.end(), ins.begin(), ins.end());
    }
    auto new_frame = Frame{f.name, instructions};
    result.push_back(new_frame);
  }
  return result;
}
} // namespace target