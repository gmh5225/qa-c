#include "st.hpp"

namespace st {
AssignmentExpression::
AssignmentExpression(std::unique_ptr<PrimaryExpression> lhs, std::unique_ptr<PrimaryExpression> rhs)
    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
}