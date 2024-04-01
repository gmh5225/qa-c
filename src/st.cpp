#include "st.hpp"

namespace st {
AssignmentExpression::AssignmentExpression(Expression lhs, Expression rhs)
    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

AdditiveExpression::AdditiveExpression(Expression lhs, Expression rhs,
                                       AdditiveExpressionType _type)
    : lhs(std::move(lhs)), rhs(std::move(rhs)), type(_type) {}

Expression::Expression(std::unique_ptr<PrimaryExpression> expr)
    : expr(std::move(expr)) {}
Expression::Expression(std::unique_ptr<AssignmentExpression> expr)
    : expr(std::move(expr)) {}
Expression::Expression(std::unique_ptr<UnaryExpression> expr)
    : expr(std::move(expr)) {}

Expression::Expression(std::unique_ptr<AdditiveExpression> expr)
    : expr(std::move(expr)) {}

} // namespace st
