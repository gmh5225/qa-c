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

void MoveInstruction(const target::Mov mov, Ctx &ctx) {
    if (std::get<target::HardcodedRegister>(mov.dst) ==
            std::get<target::HardcodedRegister>(mov.src))
        return;
    ctx.AddInstruction("mov " + target::to_asm(mov));
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
    } else if (std::holds_alternative<target::StoreI>(is)) {
        const auto storeI = std::get<target::StoreI>(is);
        const auto sourcesizeString = std::string("dword");
        ctx.AddInstruction("mov " + sourcesizeString + " [rbp - " +
                           std::to_string(storeI.dst.offset) + "], " +
                           std::to_string(storeI.value));
    } else if (std::holds_alternative<target::Store>(is)) {
        const auto store = std::get<target::Store>(is);
        const auto src = std::get<target::HardcodedRegister>(store.src);
        const auto sourcesizeString =
            src.size == 4 ? std::string("dword") : std::string("qword");
        ctx.AddInstruction("mov " + sourcesizeString + " [rbp - " +
                           std::to_string(store.dst.offset) + "], " +
                           target::to_asm(src.reg, src.size));
    } else if (std::holds_alternative<target::Load>(is)) {
        const auto load = std::get<target::Load>(is);
        const auto dst = std::get<target::HardcodedRegister>(load.dst);
        const auto sourcesizeString =
            dst.size == 4 ? std::string("dword") : std::string("qword");
        ctx.AddInstruction("mov " + target::to_asm(dst.reg, dst.size) + ", " +
                           sourcesizeString + " [rbp - " +
                           std::to_string(load.src.offset) + "]");
    } else if (std::holds_alternative<target::AddI>(is)) {
        const auto addI = std::get<target::AddI>(is);
        const auto dst = std::get<target::HardcodedRegister>(addI.dst);
        ctx.AddInstruction("add " + target::to_asm(dst.reg, dst.size) + ", " +
                           std::to_string(addI.value));
    } else if (std::holds_alternative<target::Add>(is)) {
        const auto add = std::get<target::Add>(is);
        const auto dst = std::get<target::HardcodedRegister>(add.dst);
        const auto src = std::get<target::HardcodedRegister>(add.src);
        ctx.AddInstruction("add " + target::to_asm(dst.reg, dst.size) + ", " +
                           target::to_asm(src.reg, src.size));
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