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
    std::set<size_t> usedRegs = {};
    std::map<target::Register, target::BaseRegister> mapping = {};

    target::BaseRegister getReg() {
        std::cout << "get reg called" << std::endl;
        size_t i = 0;
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
        std::cout << "free reg called" << std::endl;
        for (size_t i = 0; i < target::general_regs.size(); i++) {
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

[[nodiscard]] FirstLastUse getFirstUse(const target::Frame &frame) {
    FirstLastUse fl = {};
    int i = 0;
    for (const auto &ins : frame.instructions) {
        ++i;
        const auto operation = ins;
        std::optional<int> srcId = get_src_virtual_id_if_present(operation);
        std::optional<int> dstId = get_dest_virtual_id_if_present(operation);
        for (auto id : {
                    srcId, dstId
                }) {
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
        auto register_src = target::get_src_register(operation);
        if (register_src.has_value()) {
            src = *register_src;
            if (newFirstUsed.find(*src) == newFirstUsed.end()) {
                newFirstUsed[*src] = idx;
            }
        }
        const auto register_dest = target::get_dest_register(operation);
        if (register_dest.has_value()) {
            const auto dest = *register_dest;
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

void flag_register_pressure(std::map<int, int> firstUse,
                            std::map<int, int> lastUse, int instructions) {
    int instructionsCurrentlyLive = 0;
    int instructionsCurrentlyLiveMax = 0;
    int instructionAtLiveMaxLocation = 0;
    for (int instructionLocation = 0; instructionLocation < instructions;
            instructionLocation++) {
        for (const auto &entry : firstUse) {
            const auto firstUseLocation = entry.second;
            const auto lastUseLocation = lastUse[entry.first];
            if (firstUseLocation == instructionLocation) {
                instructionsCurrentlyLive += 1;
            }
            if (lastUseLocation == instructionLocation) {
                instructionsCurrentlyLive -= 1;
            }
            if (instructionsCurrentlyLive > instructionsCurrentlyLiveMax) {
                instructionsCurrentlyLiveMax = instructionsCurrentlyLive;
                instructionAtLiveMaxLocation = instructionLocation;
            }
        }
    }
    std::cout << "Max instructions live at once: " << instructionsCurrentlyLiveMax
              << " at instruction " << instructionAtLiveMaxLocation << std::endl;
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
    flag_register_pressure(firstUse, lastUse, frame.instructions.size());
    std::vector<target::Instruction> newInstructions = {};
    int idx = 0;
    for (auto &i : frame.instructions) {
        idx += 1;
        auto operation = i;
        std::cout << operation << std::endl;
        auto src_op = target::get_src_register(operation);
        if (src_op.has_value()) {
            auto src = src_op.value();
            if (lastUse.find(src.id) == lastUse.end()) {
                throw std::runtime_error("Last use not found");
            }
            if (remappedRegisters.find(src) != remappedRegisters.end()) {
                const auto remapped = remappedRegisters.at(src);
                src = remapped;
            }
            if (firstUse[src.id] == idx) {
                ctx.mapping[src] = ctx.getReg();
            }
            if (ctx.mapping.find(src) == ctx.mapping.end()) {
                ctx.mapping[src] = ctx.getReg();
            }
            target::set_src_register(
                operation, target::HardcodedRegister{ctx.mapping.at(src), src.size});
            if (lastUse[src.id] <= idx) {
                ctx.freeReg(ctx.mapping.at(src));
            }
        }
        auto dest_op = get_dest_register(operation);
        if (dest_op.has_value()) {
            auto dest = dest_op.value();
            if (remappedRegisters.find(dest) != remappedRegisters.end()) {
                const auto remapped = remappedRegisters.at(dest);
                dest = remapped;
            }
            if (firstUse[dest.id] == idx) {
                ctx.mapping[dest] = ctx.getReg();
            }
            if (ctx.mapping.find(dest) == ctx.mapping.end()) {
                ctx.mapping[dest] = ctx.getReg();
            }
            target::set_dest_register(
                operation,
                target::HardcodedRegister{ctx.mapping.at(dest), dest.size});
            if (lastUse[dest.id] <= idx) {
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
