#include <stdexcept>

#include "operation.hpp"

namespace qa_ir {

std::ostream &operator<<(std::ostream &os, const Operation &ins) {
  std::visit([&os](const auto &arg) { os << arg; }, ins);
  return os;
}

} // namespace qa_ir