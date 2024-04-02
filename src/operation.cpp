#include <stdexcept>

#include "operation.hpp"

namespace qa_ir {

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
    } else {
        throw std::runtime_error("Unknown instruction type");
    }
    return os;
}

} // namespace qa_ir