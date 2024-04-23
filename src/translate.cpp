
#include "../include/translate.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <variant>

#include "../include/asttraits.hpp"
namespace ast {

// primary
auto translate(const std::unique_ptr<st::PrimaryExpression>& expr, Ctx& ctx)
    -> std::unique_ptr<AstNode> {
    if (expr->type == st::PrimaryExpressionType::INT) {
        return std::make_unique<ConstIntAstNode>(expr->value);
    }
    if (expr->type == st::PrimaryExpressionType::IDEN) {
        const auto& iden = expr->idenValue;
        if (ctx.local_variables.find(iden) != ctx.local_variables.end()) {
            const auto& var = ctx.local_variables[iden];
            return std::make_unique<VariableAstNode>(iden, var->variableType);
        }
        throw std::runtime_error("Variable not found: " + iden);
    }
    throw std::runtime_error(
        "translate(st::PrimaryExpression *expr, Ctx &ctx) not implemented");
}

// assignment
[[nodiscard]] std::unique_ptr<AstNode> translate(
    const std::unique_ptr<st::AssignmentExpression>& expr, Ctx& ctx) {
    ctx.set_lvalueContext(
        "translate(const st::AssignmentExpression &expr, Ctx &ctx)", true);
    auto lhs = translate(expr->lhs, ctx);
    ctx.set_lvalueContext(
        "translate(const st::AssignmentExpression &expr, Ctx &ctx)", false);
    auto rhs = translate(expr->rhs, ctx);
    return std::make_unique<MoveAstNode>(std::move(lhs), std::move(rhs));
}

// unary expression
// assignment
[[nodiscard]] auto translate(const std::unique_ptr<st::UnaryExpression>& expr,
                             Ctx& ctx) -> std::unique_ptr<AstNode> {
    auto e = translate(expr->expr, ctx);
    if (expr->type == st::UnaryExpressionType::DEREF &&
        ctx.__lvalueContext == false) {
        return std::make_unique<DerefReadAstNode>(std::move(e));
    } else if (expr->type == st::UnaryExpressionType::DEREF &&
               ctx.__lvalueContext == true) {
        return std::make_unique<DerefWriteAstNode>(std::move(e));
    } else if (expr->type == st::UnaryExpressionType::ADDR) {
        return std::make_unique<AddrAstNode>(std::move(e));
    } else if (expr->type == st::UnaryExpressionType::NEG) {
        auto zero = std::make_unique<ConstIntAstNode>(0);
        return std::make_unique<BinaryOpAstNode>(std::move(zero), std::move(e),
                                                 BinOpKind::Sub);
    }

    throw std::runtime_error(
        "translate(const st::UnaryExpression &expr, Ctx &ctx)");
    std::unreachable();
}

[[nodiscard]] std::unique_ptr<AstNode> translate(
    const std::unique_ptr<st::AdditiveExpression>& expr, Ctx& ctx) {
    auto lhs = translate(expr->lhs, ctx);
    auto rhs = translate(expr->rhs, ctx);
    std::unordered_map<st::AdditiveExpressionType, BinOpKind> mp = {
        {st::AdditiveExpressionType::ADD, BinOpKind::Add},
        {st::AdditiveExpressionType::SUB, BinOpKind::Sub},
        {st::AdditiveExpressionType::EQ, BinOpKind::Eq},
        {st::AdditiveExpressionType::NEQ, BinOpKind::Neq},
        {st::AdditiveExpressionType::GT, BinOpKind::Gt},
        {st::AdditiveExpressionType::LT, BinOpKind::Lt},
    };
    if (mp.find(expr->type) != mp.end()) {
        return std::make_unique<BinaryOpAstNode>(std::move(lhs), std::move(rhs),
                                                 mp[expr->type]);
    }
    throw std::runtime_error(
        "translate(const st::AdditiveExpression &expr, Ctx &ctx)");
}

auto translate(const std::unique_ptr<st::ForStatement>& stmt, Ctx& ctx)
    -> std::unique_ptr<AstNode> {
    const st::ForDeclaration& init = stmt->init;
    const auto iden =
        init.initDeclarator.value().declarator.directDeclarator.VariableIden();
    auto datatype = asttraits::toDataType(init);
    auto var = std::make_unique<VariableAstNode>(iden, *datatype);
    ctx.local_variables[iden] = var.get();
    const auto& expr = init.initDeclarator.value().initializer.value().expr;
    ctx.set_lvalueContext(
        "translate(const std::unique_ptr<st::ForStatement> &stmt, Ctx &ctx)",
        false);
    auto initInFirstEntryOfForLoop = translate(expr, ctx);
    ctx.set_lvalueContext(
        "translate(const std::unique_ptr<st::ForStatement> &stmt, Ctx &ctx)",
        true);
    auto forInit = std::make_unique<MoveAstNode>(
        std::move(var), std::move(initInFirstEntryOfForLoop));

    std::optional<std::unique_ptr<AstNode>> forCondition;
    std::optional<std::unique_ptr<AstNode>> forUpdate;
    if (stmt->cond) {
        forCondition = translate(*stmt->cond, ctx);
    }
    if (stmt->inc) {
        forUpdate = translate(*stmt->inc, ctx);
    }
    if (stmt->body == nullptr) {
        throw std::runtime_error("for body is null");
    }
    auto body = translate(*stmt->body.get(), ctx);
    auto result = std::make_unique<ForLoopAstNode>(
        std::move(forInit), std::move(forCondition), std::move(forUpdate),
        std::move(body));
    return result;
}

auto translate(const std::unique_ptr<st::FunctionCallExpression>& expr,
               Ctx& ctx) -> std::unique_ptr<AstNode> {
    std::vector<std::unique_ptr<AstNode>> args;
    for (const auto& arg : expr->args) {
        auto e = translate(arg, ctx);
        args.push_back(std::move(e));
    }

    const auto faux_return_type = ast::DataType("int", 4, nullptr);

    return std::make_unique<FunctionCallAstNode>(expr->name, std::move(args),
                                                 faux_return_type);
}

// expression
[[nodiscard]] std::unique_ptr<AstNode> translate(const st::Expression& expr,
                                                 Ctx& ctx) {
    return std::visit(
        [&ctx](auto&& arg) { return translate(std::move(arg), ctx); }, expr);
}

// return statement
[[nodiscard]] std::unique_ptr<AstNode> translate(
    const std::unique_ptr<st::ReturnStatement>& stmt, Ctx& ctx) {
    ctx.set_lvalueContext(
        "translate(const st::ReturnStatement &stmt, Ctx &ctx)", false);
    auto expr = translate(stmt->expr, ctx);
    ctx.set_lvalueContext(
        "translate(const st::ReturnStatement &stmt, Ctx &ctx)", true);
    return std::make_unique<ReturnAstNode>(std::move(expr));
}

// expression statement
[[nodiscard]] std::unique_ptr<AstNode> translate(
    const std::unique_ptr<st::ExpressionStatement>& stmt, Ctx& ctx) {
    return translate(stmt->expr, ctx);
}

// selection statement statement
auto translate(const std::unique_ptr<st::SelectionStatement>& stmt, Ctx& ctx)
    -> std::unique_ptr<AstNode> {
    auto condition = translate(stmt->cond, ctx);
    if (const auto kind = condition->get_bin_op()) {
        if (!is_comparison(*kind)) {
            condition = std::make_unique<BinaryOpAstNode>(
                std::move(condition), std::make_unique<ConstIntAstNode>(0),
                BinOpKind::Neq);
        }
        auto then = translate(*stmt->then, ctx);
        if (stmt->else_) {
            auto else_ = translate(*stmt->else_, ctx);
            return std::make_unique<IfNode>(std::move(condition),
                                            std::move(then), std::move(else_));
        }
        return std::make_unique<IfNode>(std::move(condition), std::move(then),
                                        std::nullopt);
    }
}

// declaration
[[nodiscard]] auto translate(const st::Declaration& decl, Ctx& ctx)
    -> std::unique_ptr<AstNode> {
    const auto iden =
        decl.initDeclarator.value().declarator.directDeclarator.VariableIden();
    auto datatype = asttraits::toDataType(decl);
    auto var = std::make_unique<VariableAstNode>(iden, *datatype);
    ctx.local_variables[iden] = var.get();
    const auto& expr = decl.initDeclarator.value().initializer.value().expr;
    ctx.set_lvalueContext("translate(const st::Declaration &decl, Ctx &ctx)",
                          false);
    auto init = translate(expr, ctx);
    ctx.set_lvalueContext("translate(const st::Declaration &decl, Ctx &ctx)",
                          true);
    return std::make_unique<MoveAstNode>(std::move(var), std::move(init));
}

[[nodiscard]] std::vector<FrameParam> translate(st::ParamTypeList& params) {
    std::vector<FrameParam> result;
    for (const auto& p : params.params) {
        const auto name = p.Name();
        const auto type = asttraits::toDataType(p);
        const auto fp = FrameParam{
            .name = name,
            .type = *type,
        };
        result.push_back(fp);
    }
    return result;
}

auto translateStatement(st::Statement& stmt, Ctx& ctx)
    -> std::unique_ptr<AstNode> {
    auto s = std::move(stmt.stmt);
    return std::visit(
        [&ctx](auto&& arg) { return translate(std::move(arg), ctx); }, s);
}

auto translate(st::CompoundStatement& stmts, Ctx& ctx)
    -> std::vector<std::unique_ptr<AstNode>> {
    if (stmts.items.empty()) {
        return {};
    }
    std::vector<std::unique_ptr<AstNode>> result;
    for (auto& bi : stmts.items) {
        if (std::holds_alternative<st::Statement>(bi.item)) {
            auto stmt = std::get<st::Statement>(std::move(bi.item));
            auto node = translateStatement(stmt, ctx);
            result.push_back(std::move(node));
        } else {
            auto& decl = std::get<st::Declaration>(bi.item);
            auto node = translate(decl, ctx);
            result.push_back(std::move(node));
        }
    }
    return result;
}

[[nodiscard]] static std::unique_ptr<AstNode> translate(st::FuncDef* fd,
                                                        Ctx& ctx) {
    const auto functionName = fd->Name();
    auto functionParams = fd->DirectDeclarator().params;
    auto params = translate(functionParams);
    for (const auto& p : params) {
        const auto paramName = p.name;
        const auto type = p.type;
        auto var = std::make_unique<VariableAstNode>(paramName, type);
        ctx.local_variables[p.name] = var.get();
    }
    std::vector<std::unique_ptr<AstNode>> body = translate(fd->body, ctx);
    return std::make_unique<FrameAstNode>(functionName, std::move(body),
                                          std::move(params));
}

[[nodiscard]] static std::unique_ptr<AstNode> translate(
    const st::ExternalDeclaration& node, Ctx& ctx) {
    const auto& nv = node.node;
    if (std::holds_alternative<std::unique_ptr<st::FuncDef>>(nv)) {
        auto funcdef = std::get<std::unique_ptr<st::FuncDef>>(nv).get();
        return translate(funcdef, ctx);
    }
    const auto& decl = std::get<st::Declaration>(nv);
    return translate(decl, ctx);
}

[[nodiscard]] std::vector<std::unique_ptr<AstNode>> translate(
    const st::Program& program) {
    auto ctx = Ctx{
        .counter = 0,
        .local_variables = {},
    };
    std::vector<std::unique_ptr<AstNode>> nodes;
    for (const auto& decl : program.nodes) {
        auto node = translate(decl, ctx);
        nodes.push_back(std::move(node));
    }
    return nodes;
}
}  // namespace ast
