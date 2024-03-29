#include <iostream>

#include "codegen.hpp"

namespace codegen {
class Ctx {
  public:
    std::string Code;

    void AddInstructionNoIndent(const std::string &i) {
        Code += (i + "\n");
    }

    void AddInstruction(const std::string &i) {
        Code += "\t" + i + "\n";
    }
};

void generateASMForInstruction(const as::Instruction &is, Ctx &ctx) {
    if (std::holds_alternative<as::Operation>(is)) {
        const auto &op = std::get<as::Operation>(is);
        if (op.op == as::OpCode::Mov) {
            auto dest = as::toAsm(op.dest);
            auto src = as::toAsm(op.src);
            ctx.AddInstruction("mov " + dest + ", " + src);
        } else if (op.op == as::OpCode::LoadI) {
            auto dest = as::toAsm(op.dest);
            ctx.AddInstruction("mov " + dest + ", " +
                               std::to_string(op.value.value()));
        }
    } else if (std::holds_alternative<as::Label>(is)) {
        const auto &label = std::get<as::Label>(is);
        ctx.AddInstructionNoIndent(label.name + ":");
    }
}

int sixteenByteAlign(int size) {
    return size % 16 == 0 ? size : size + (16 - (size % 16));
}

void generateASMForFrame(const as::Frame &frame, Ctx &ctx) {
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

[[nodiscard]] std::string Generate(const std::vector<as::Frame> &frames) {
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