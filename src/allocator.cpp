#include "allocator.hpp"
#include <set>
#include <stdexcept>
#include <map>
#include <variant>
#include <unordered_map>
#include <vector>
#include <optional>

namespace allocator {
using reg = std::variant<as::Temp, as::HardcodedRegister>;
struct Ctx {
  public:
    std::set<int> usedRegs = {};
    std::map<reg, std::string> mapping = {};
    std::vector<std::string> general_regs = {"ax", "bx", "cx", "dx", "si", "di", "8", "9", "10", "11", "12", "13", "14", "15"};

    std::string getReg() {
        int i = 0;
        for (const auto &str : general_regs) {
            if (usedRegs.find(i) == usedRegs.end()) {
                usedRegs.insert(i);
                return str;
            }
            i++;
        }
        throw std::runtime_error("No free registers");
    }

    std::string freeReg(std::string reg) {
        int i = 0;
        for (const auto &str : general_regs) {
            if (str == reg) {
                usedRegs.erase(i);
                return str;
            }
            i++;
        }
        throw std::runtime_error("Register not found");
    }

};

struct FirstLastUse {
    std::map<as::Temp, int> firstUse = {};
    std::map<as::Temp, int> lastUse = {};
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
            if (fl.firstUse.find(temp) == fl.firstUse.end()) {
                fl.firstUse[temp] = i;
            }
            fl.lastUse[temp] = i;
        }
        if (std::holds_alternative<as::Temp>(operation.dest)) {
            const auto temp = std::get<as::Temp>(operation.dest);
            if (fl.firstUse.find(temp) == fl.firstUse.end()) {
                fl.firstUse[temp] = i;
            }
            fl.lastUse[temp] = i;
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
            if (operation.op != as::OpCode::Mov) continue;
            // if not found in newFirstused, (ie first used as a dest) then we need to remap
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
    const auto remappedRegisters = remap(newFrame);
    for (const auto &entry : remappedRegisters) {
        const auto prev = entry.first;
        const auto newReg = entry.second;
        firstUse[newReg] = std::min(firstUse[newReg], firstUse[prev]);
        lastUse[newReg] = std::max(lastUse[newReg], lastUse[prev]);
    }
    std::vector<as::Instruction> newInstructions = {};
    int idx = 0;
    for (const auto &i : newFrame.instructions) {
        idx += 1;
        if (std::holds_alternative<as::Label>(i)) {
            newInstructions.push_back(i);
            continue;
        }
        auto operation = std::get<as::Operation>(i);
        std::optional<as::Location> src;
        std::optional<as::Location> dest;
        if (auto __src = std::get_if<as::HardcodedRegister>(&operation.src)) {
            src = *__src;
        }
        if (auto __src = std::get_if<as::Temp>(&operation.src)) {
            src = *__src;
            if (remappedRegisters.find(*__src) != remappedRegisters.end()) {
                src = remappedRegisters.at(*__src);
            }
            if (firstUse[*__src] == idx) {
                const auto reg = ctx.getReg();
                ctx.mapping[*__src] = reg;
            }
            const auto register_string = ctx.mapping.at(*__src);
            src = as::HardcodedRegister{register_string, as::SizeOf(*src)};
            operation.src = *src;
            if (lastUse[*__src] == idx) {
                ctx.freeReg(ctx.mapping.at(*__src));
            }
        }
        if (auto __dest = std::get_if<as::Temp>(&operation.dest)) {
            dest = *__dest;
            if (remappedRegisters.find(*__dest) != remappedRegisters.end()) {
                dest = remappedRegisters.at(*__dest);
            }
            if (firstUse[*__dest] == idx) {
                const auto reg = ctx.getReg();
                ctx.mapping[*__dest] = reg;
            }
            if (ctx.mapping.find(*__dest) == ctx.mapping.end()) {
                if (std::holds_alternative<as::Temp>(*dest)) {
                    ctx.mapping[*__dest] = ctx.getReg();
                } else {
                    ctx.mapping[*__dest] = std::get<as::HardcodedRegister>(*dest).name.substr(1);
                }
            }
            const auto register_string = ctx.mapping.at(*__dest);
            dest = as::HardcodedRegister{register_string, as::SizeOf(*dest)};
            operation.dest = *dest;
            if (lastUse[*__dest] == idx) {
                ctx.freeReg(ctx.mapping.at(*__dest));
            }
        }
        newInstructions.push_back(operation);
    }
    newFrame.instructions = newInstructions;
    return newFrame;
}

[[nodiscard]] std::vector<as::Frame> rewrite(const std::vector<as::Frame> &frames) {
    std::vector<as::Frame> newFrames;
    for (const auto &frame : frames) {
        Ctx ctx = Ctx{};
        newFrames.push_back(rewrite(frame, ctx));
    }
    return newFrames;
}
} // namespace allocator
