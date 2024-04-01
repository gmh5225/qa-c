#include <iostream>

#include "codegen.hpp"

namespace codegen {
class Ctx {
public:
  std::string Code;

  void AddInstructionNoIndent(const std::string &i) { Code += (i + "\n"); }

  void AddInstruction(const std::string &i) { Code += "\t" + i + "\n"; }
};

void MoveInstruction(const target::Mov mov, Ctx &ctx) {
  const auto src = mov.src_reg();
  const auto dest = mov.dst_reg();
  ctx.AddInstruction("mov " + target::to_asm(dest.reg, dest.size) + ", " +
                     target::to_asm(src.reg, src.size));
}

void generateASMForInstruction(const target::Instruction &is, Ctx &ctx) {
  if (std::holds_alternative<target::Mov>(is)) {
    MoveInstruction(std::get<target::Mov>(is), ctx);
  } else if (std::holds_alternative<target::Jump>(is)) {
    const auto jump = std::get<target::Jump>(is);
    ctx.AddInstruction("jmp ." + jump.label);
  } else if (std::holds_alternative<target::LoadI>(is)) {
    const auto loadI = std::get<target::LoadI>(is);
    const auto dst = std::get<target::HardcodedRegister>(loadI.dst);
    ctx.AddInstruction("mov " + target::to_asm(dst.reg, dst.size) + ", " +
                       std::to_string(loadI.value));
  } else {
    throw std::runtime_error("Unsupported instruction type" +
                             std::to_string(is.index()));
  }
}

int sixteenByteAlign(int size) {
  return size % 16 == 0 ? size : size + (16 - (size % 16));
}

void generateASMForFrame(const target::Frame &frame, Ctx &ctx) {
  ctx.AddInstructionNoIndent(frame.name + ":");
  ctx.AddInstruction("push rbp");
  ctx.AddInstruction("mov rbp, rsp");
  // ctx.AddInstruction("sub rsp, " +
  // std::to_string(sixteenByteAlign(frame.size)));
  for (const auto &is : frame.instructions) {
    try {
      generateASMForInstruction(is, ctx);
    } catch (const std::exception &e) {
      std::cerr << "Error generating ASM for operation: " << e.what()
                << std::endl;
      throw;
    }
  }
  ctx.AddInstructionNoIndent(".end:");
  if (frame.size > 0) {
    ctx.AddInstruction("leave");
  } else {
    ctx.AddInstruction("pop rbp");
  }
  ctx.AddInstruction("ret");
}

[[nodiscard]] std::string Generate(const std::vector<target::Frame> &frames) {
  Ctx ctx;
  ctx.AddInstructionNoIndent("section .text");
  ctx.AddInstructionNoIndent("global _start");
  for (const auto &frame : frames) {
    generateASMForFrame(frame, ctx);
  }
  ctx.AddInstructionNoIndent("_start:");
  ctx.AddInstruction("call main");
  ctx.AddInstruction("mov edi, eax");
  ctx.AddInstruction("mov eax, 60");
  ctx.AddInstruction("syscall");
  return ctx.Code;
}
} // namespace codegen