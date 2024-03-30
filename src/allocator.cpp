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
using reg = std::variant<as::Temp, as::HardcodedRegister>;
struct Ctx {
  public:
    std::set<int> usedRegs = {};
    std::map<reg, target::BaseRegister> mapping = {};

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

[[nodiscard]] FirstLastUse getFirstUse(const as::Frame &frame) {
    FirstLastUse fl = {};
    int i = 0;
    for (const auto &ins : frame.instructions) {
        i += 1;
        if (std::holds_alternative<as::Label>(ins)) {
            continue;
        }
        const auto operation = std::get<as::Operation>(ins);
        if (std::holds_alternative<as::Temp>(operation.src)) {
            const auto temp = std::get<as::Temp>(operation.src);
            if (fl.firstUse.find(temp.id) == fl.firstUse.end()) {
                fl.firstUse[temp.id] = i;
            }
            fl.lastUse[temp.id] = i;
        }
        if (std::holds_alternative<as::Temp>(operation.dest)) {
            const auto temp = std::get<as::Temp>(operation.dest);
            if (fl.firstUse.find(temp.id) == fl.firstUse.end()) {
                fl.firstUse[temp.id] = i;
            }
            fl.lastUse[temp.id] = i;
        }
    }
    return fl;
}

[[nodiscard]] std::map<as::Temp, as::Temp> remap(as::Frame &frame) {
    std::map<as::Temp, as::Temp> remappedRegisters = {};
    std::map<as::Temp, int> newFirstUsed = {};
    int idx = 0;
    for (const auto &i : frame.instructions) {
        idx += 1;
        if (std::holds_alternative<as::Label>(i)) {
            continue;
        }
        auto operation = std::get<as::Operation>(i);
        std::optional<as::Temp> src;
        if (auto __src = std::get_if<as::Temp>(&operation.src)) {
            src = *__src;
            if (newFirstUsed.find(*src) == newFirstUsed.end()) {
                newFirstUsed[*src] = idx;
            }
        }
        if (auto dest = std::get_if<as::Temp>(&operation.dest)) {
            if (operation.op != as::OpCode::Mov)
                continue;
            // if not found in newFirstused, (ie first used as a dest) then we need to
            // remap
            if (newFirstUsed.find(*dest) == newFirstUsed.end()) {
                if (src.has_value()) {
                    // if src is in remapped registers, then we need to remap it
                    if (remappedRegisters.find(*src) != remappedRegisters.end()) {
                        remappedRegisters[*dest] = remappedRegisters[*src];
                    } else {
                        remappedRegisters[*dest] = *src;
                    }
                }
            }
        }
    }
    return remappedRegisters;
}

[[nodiscard]] as::Frame rewrite(const as::Frame &frame, Ctx &ctx) {
    as::Frame newFrame = frame;
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
    std::vector<as::Instruction> newInstructions = {};
    int idx = 0;
    for (auto &i : newFrame.instructions) {
        idx += 1;
        if (std::holds_alternative<as::Label>(i)) {
            newInstructions.push_back(i);
            continue;
        }
        auto operation = std::get<as::Operation>(i);
        if (std::holds_alternative<as::Temp>(operation.src)) {
            const auto src = std::get<as::Temp>(operation.src);
            if (lastUse.find(src.id) == lastUse.end()) {
                throw std::runtime_error("Last use not found");
            }
            if (remappedRegisters.find(src) != remappedRegisters.end()) {
                operation.src = remappedRegisters.at(src);
            }
            if (firstUse[src.id] == idx) {
                ctx.mapping[src] = ctx.getReg();
            }
            if (ctx.mapping.find(src) == ctx.mapping.end()) {
                ctx.mapping[src] = ctx.getReg();
            }
            operation.src =
                as::HardcodedRegister{ctx.mapping.at(src), as::SizeOf(src)};
            if (lastUse[src.id] == idx) {
                ctx.freeReg(ctx.mapping.at(src));
            }
        }
        if (std::holds_alternative<as::Temp>(operation.dest)) {
            const auto dest = std::get<as::Temp>(operation.dest);
            if (remappedRegisters.find(dest) != remappedRegisters.end()) {
                operation.dest = remappedRegisters.at(dest);
            }
            if (firstUse[dest.id] == idx) {
                ctx.mapping[dest] = ctx.getReg();
            }
            if (ctx.mapping.find(dest) == ctx.mapping.end()) {
                ctx.mapping[dest] = ctx.getReg();
            }
            operation.dest =
                as::HardcodedRegister{ctx.mapping.at(dest), as::SizeOf(dest)};
            if (lastUse[dest.id] == idx) {
                ctx.freeReg(ctx.mapping.at(dest));
            }
        }
        newInstructions.push_back(operation);
    }
    newFrame.instructions = newInstructions;
    return newFrame;
}

[[nodiscard]] std::vector<as::Frame>
rewrite(const std::vector<as::Frame> &frames) {
    std::vector<as::Frame> newFrames;
    for (const auto &frame : frames) {
        Ctx ctx = Ctx{};
        newFrames.push_back(rewrite(frame, ctx));
    }
    return newFrames;
}
} // namespace allocator
