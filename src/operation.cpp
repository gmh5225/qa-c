#include <stdexcept>

#include "operation.hpp"

namespace qa_ir {

std::ostream &operator<<(std::ostream &os, const Label &label) {
    os << label.name;
    return os;
}

std::ostream &operator<<(std::ostream &os, const Operation &ins) {
    if (std::holds_alternative<Mov>(ins)) {
        const auto &mov = std::get<Mov>(ins);
        os << "mov " << mov.dst << ", " << mov.src;
    } else if (std::holds_alternative<Ret>(ins)) {
        const auto &ret = std::get<Ret>(ins);
        os << "ret " << ret.value;
    } else if (std::holds_alternative<Add>(ins)) {
        const auto &add = std::get<Add>(ins);
        os << "add " << add.dst << ", " << add.left << ", " << add.right;
    } else if (std::holds_alternative<Sub>(ins)) {
        const auto &sub = std::get<Sub>(ins);
        os << "sub " << sub.dst << ", " << sub.left << ", " << sub.right;
    } else if (std::holds_alternative<MovR>(ins)) {
        const auto &movr = std::get<MovR>(ins);
        os << "mov " << movr.dst << ", " << movr.src;
    } else if (std::holds_alternative<Addr>(ins)) {
        const auto &addr = std::get<Addr>(ins);
        os << "addr " << addr.dst << ", " << addr.src;
    } else if (std::holds_alternative<Deref>(ins)) {
        const auto &deref = std::get<Deref>(ins);
        os << "deref " << deref.dst << ", " << deref.src;
    } else if (std::holds_alternative<StoreAddr>(ins)) {
        const auto &storeaddr = std::get<StoreAddr>(ins);
        os << "storeaddr " << storeaddr.dst << ", " << storeaddr.src;
    } else if (std::holds_alternative<Equal>(ins)) {
        const auto &equal = std::get<Equal>(ins);
        os << "equal " << equal.dst << ", " << equal.left << ", " << equal.right;
    } else if (std::holds_alternative<ConditionalJump>(ins)) {
        const auto &cj = std::get<ConditionalJump>(ins);
        os << "cj " << cj.trueLabel << ", " << cj.falseLabel;
    } else if (std::holds_alternative<LabelDef>(ins)) {
        const auto &label = std::get<LabelDef>(ins);
        os << label.label << ":";
    } else if (std::holds_alternative<Compare>(ins)) {
        const auto &cmp = std::get<Compare>(ins);
        os << "cmp " << cmp.left << ", " << cmp.right;
    } else {
        throw std::runtime_error("Unknown instruction type");
    }
    return os;
}

} // namespace qa_ir