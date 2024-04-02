#include "optimizer.hpp"
namespace optimizer {
[[nodiscard]] target::Frame optimize_lowered(const target::Frame &frame) {
    target::Frame newFrame = frame;
    std::vector<target::Instruction> newInstructions;
    for (auto &ins : frame.instructions) {
        newInstructions.push_back(ins);
    }
    newFrame.instructions = newInstructions;
    newFrame.name = frame.name;
    return newFrame;
}

[[nodiscard]] std::vector<target::Frame>
optimize_lowered(const std::vector<target::Frame> &frames) {
    std::vector<target::Frame> newFrames;
    for (const auto &frame : frames) {
        newFrames.push_back(optimize_lowered(frame));
    }
    return newFrames;
}

[[nodiscard]] qa_ir::Frame optimize(const qa_ir::Frame &frame) {
    qa_ir::Frame newFrame = frame;
    std::vector<qa_ir::Operation> newInstructions;
    for (auto &ins : frame.instructions) {
        newInstructions.push_back(ins);
    }
    newFrame.instructions = newInstructions;
    return newFrame;
}

[[nodiscard]] std::vector<qa_ir::Frame>
optimize(const std::vector<qa_ir::Frame> &frames) {
    std::vector<qa_ir::Frame> newFrames;
    for (const auto &frame : frames) {
        newFrames.push_back(optimize(frame));
    }
    return newFrames;
}
} // namespace optimizer
