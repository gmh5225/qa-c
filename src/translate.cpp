
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <variant>

#include "asttraits.hpp"
#include "translate.hpp"

struct Ctx {
    unsigned long counter = 0;
    bool __lvalueContext = false;

    std::unordered_map<std::string, ast::Node *> local_variables;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

    void set_lvalueContext(std::string why, bool value) {
        // std::cout << "set_lvalueContext: " << why << " " << value << std::endl;
        __lvalueContext = value;
    }
#pragma clang diagnostic pop
};

[[nodiscard]] std::unique_ptr<ast::Node> translate(const st::Expression &expr,
        Ctx &ctx);
[[nodiscard]] static std::vector<std::unique_ptr<ast::Node>>
        translate(st::CompoundStatement &stmts, Ctx &ctx);

// primary
[[nodiscard]] std::unique_ptr<ast::Node>
translate(const std::unique_ptr<st::PrimaryExpression> &expr, Ctx &ctx) {
    if (expr->type == st::PrimaryExpressionType::INT) {
        return ast::makeConstInt(expr->value);
    }
    if (expr->type == st::PrimaryExpressionType::IDEN) {
        const auto &iden = expr->idenValue;
        if (ctx.local_variables.find(iden) != ctx.local_variables.end()) {
            const auto &var = ctx.local_variables[iden];
            return ast::makeNewVar(iden, var->variableType);
        }
        throw std::runtime_error("Variable not found: " + iden);
    }
    throw std::runtime_error(
        "translate(st::PrimaryExpression *expr, Ctx &ctx) not implemented");
}

// assignment
[[nodiscard]] std::unique_ptr<ast::Node>
translate(const std::unique_ptr<st::AssignmentExpression> &expr, Ctx &ctx) {
    ctx.set_lvalueContext(
           "translate(const st::AssignmentExpression &expr, Ctx &ctx)", true);
    auto lhs = translate(expr->lhs, ctx);
    ctx.set_lvalueContext(
           "translate(const st::AssignmentExpression &expr, Ctx &ctx)", false);
    auto rhs = translate(expr->rhs, ctx);
    return ast::makeNewMove(std::move(lhs), std::move(rhs));
}

// unary expression
// assignment
[[nodiscard]] std::unique_ptr<ast::Node>
translate(const std::unique_ptr<st::UnaryExpression> &expr, Ctx &ctx) {
    auto e = translate(expr->expr, ctx);
    std::cout << "translated unary expression: " << *e << std::endl;
    if (expr->type == st::UnaryExpressionType::DEREF &&
            ctx.__lvalueContext == false) {
        return ast::makeNewMemRead(std::move(e));
    } else if (expr->type == st::UnaryExpressionType::DEREF &&
               ctx.__lvalueContext == true) {
        return ast::makeNewMemWrite(std::move(e));
    } else if (expr->type == st::UnaryExpressionType::ADDR) {
        return ast::makeNewAddr(std::move(e));
    }
    throw std::runtime_error(
        "translate(const st::UnaryExpression &expr, Ctx &ctx)");
}

[[nodiscard]] std::unique_ptr<ast::Node>
translate(const std::unique_ptr<st::AdditiveExpression> &expr, Ctx &ctx) {
    auto lhs = translate(expr->lhs, ctx);
    auto rhs = translate(expr->rhs, ctx);
    if (expr->type == st::AdditiveExpressionType::ADD) {
        return ast::makeNewBinOp(std::move(lhs), std::move(rhs),
                                 ast::BinOpKind::Add);
    }
    if (expr->type == st::AdditiveExpressionType::SUB) {
        return ast::makeNewBinOp(std::move(lhs), std::move(rhs),
                                 ast::BinOpKind::Sub);
    }
    if (expr->type == st::AdditiveExpressionType::EQ) {
        return ast::makeNewBinOp(std::move(lhs), std::move(rhs),
                                 ast::BinOpKind::Eq);
    }
    throw std::runtime_error(
        "translate(const st::AdditiveExpression &expr, Ctx &ctx)");
}

// expression
[[nodiscard]] std::unique_ptr<ast::Node> translate(const st::Expression &expr,
        Ctx &ctx) {
    if (std::holds_alternative<std::unique_ptr<st::PrimaryExpression>>(expr)) {
        const auto &pe =
            std::get<std::unique_ptr<st::PrimaryExpression>>(std::move(expr));
        return translate(pe, ctx);
    }
    if (std::holds_alternative<std::unique_ptr<st::AssignmentExpression>>(expr)) {
        const auto &ae =
            std::get<std::unique_ptr<st::AssignmentExpression>>(std::move(expr));
        return translate(ae, ctx);
    }
    if (std::holds_alternative<std::unique_ptr<st::UnaryExpression>>(expr)) {
        const auto &ue =
            std::get<std::unique_ptr<st::UnaryExpression>>(std::move(expr));
        return translate(ue, ctx);
    }
    if (std::holds_alternative<std::unique_ptr<st::AdditiveExpression>>(expr)) {
        const auto &ae =
            std::get<std::unique_ptr<st::AdditiveExpression>>(std::move(expr));
        return translate(ae, ctx);
    }
    throw std::runtime_error("translate(const st::Expression &expr, Ctx &ctx)");
}

// return statement
[[nodiscard]] std::unique_ptr<ast::Node>
translate(const std::unique_ptr<st::ReturnStatement> &stmt, Ctx &ctx) {
    ctx.set_lvalueContext("translate(const st::ReturnStatement &stmt, Ctx &ctx)",
                          false);
    auto expr = translate(stmt->expr, ctx);
    ctx.set_lvalueContext("translate(const st::ReturnStatement &stmt, Ctx &ctx)",
                          true);
    return ast::makeNewReturn(std::move(expr));
}

// expression statement
[[nodiscard]] std::unique_ptr<ast::Node>
translate(const std::unique_ptr<st::ExpressionStatement> &stmt, Ctx &ctx) {
    return translate(stmt->expr, ctx);
}

// selection statement statement
[[nodiscard]] std::unique_ptr<ast::Node>
translate(const std::unique_ptr<st::SelectionStatement> &stmt, Ctx &ctx) {
    auto condition = translate(stmt->cond, ctx);
    auto kind = condition->binOpKind;
    if (kind != ast::BinOpKind::Eq) {
        condition = ast::makeNewBinOp(std::move(condition), ast::makeConstInt(0),
                                      ast::BinOpKind::Eq);
        auto then = translate(*stmt->then, ctx);
        // SWAP BECAUSE WE ARE DOING A NEQ
        if (stmt->else_) {
            auto else_ = translate(*stmt->else_, ctx);
            return ast::makeNewIfStmt(std::move(condition), std::move(else_),
                                      std::move(then));
        }
        return ast::makeNewIfStmt(std::move(condition), {}, std::move(then));
    }
    auto then = translate(*stmt->then, ctx);
    if (stmt->else_) {
        auto else_ = translate(*stmt->else_, ctx);
        return ast::makeNewIfStmt(std::move(condition), std::move(then),
                                  std::move(else_));
    }
    return ast::makeNewIfStmt(std::move(condition), std::move(then), {});
}

// declaration
[[nodiscard]] static std::unique_ptr<ast::Node>
translate(const st::Declaration &decl, Ctx &ctx) {
    const auto iden =
        decl.initDeclarator.value().declarator.directDeclarator.VariableIden();
    auto datatype = asttraits::toDataType(decl);
    auto var = ast::makeNewVar(iden, *datatype);
    ctx.local_variables[iden] = var.get();
    const auto &expr = decl.initDeclarator.value().initializer.value().expr;
    ctx.set_lvalueContext("translate(const st::Declaration &decl, Ctx &ctx)",
                          false);
    auto init = translate(expr, ctx);
    ctx.set_lvalueContext("translate(const st::Declaration &decl, Ctx &ctx)",
                          true);
    return ast::makeNewMove(std::move(var), std::move(init));
}

[[nodiscard]] std::vector<ast::FrameParam>
translate(st::ParamTypeList &params) {
    std::vector<ast::FrameParam> result;
    for (const auto &p : params.params) {
        const auto name = p.Name();
        const auto type = asttraits::toDataType(p);
        const auto fp = ast::FrameParam{
            .name = name,
            .type = *type,
        };
        result.push_back(fp);
    }
    return result;
}

[[nodiscard]] static std::unique_ptr<ast::Node> translate(st::Statement &stmt,
        Ctx &ctx) {
    auto s = std::move(stmt.stmt);
    return std::visit(
    [&ctx](auto &&arg) -> std::unique_ptr<ast::Node> {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::unique_ptr<st::ReturnStatement>>) {
            return translate(std::move(arg), ctx);
        } else if constexpr (std::is_same_v<
                             T, std::unique_ptr<st::ExpressionStatement>>) {
            return translate(std::move(arg), ctx);
        } else if constexpr (std::is_same_v<
                             T, std::unique_ptr<st::SelectionStatement>>) {
            return translate(std::move(arg), ctx);
        }

        else {
            throw std::runtime_error(
                "translate(const st::Statement &stmt, Ctx &ctx)");
        }
    },
    s);
}

[[nodiscard]] static std::vector<std::unique_ptr<ast::Node>>
translate(st::CompoundStatement &stmts, Ctx &ctx) {
    if (stmts.items.empty()) {
        return {};
    }
    std::vector<std::unique_ptr<ast::Node>> result;
    for (auto &bi : stmts.items) {
        if (std::holds_alternative<st::Statement>(bi.item)) {
            auto stmt = std::get<st::Statement>(std::move(bi.item));
            auto node = translate(stmt, ctx);
            result.push_back(std::move(node));
        } else {
            auto &decl = std::get<st::Declaration>(bi.item);
            auto node = translate(decl, ctx);
            result.push_back(std::move(node));
        }
    }
    return result;
}

[[nodiscard]] static std::unique_ptr<ast::Node> translate(st::FuncDef *fd,
        Ctx &ctx) {
    const auto name = fd->Name();
    auto functionParams = fd->DirectDeclarator().params;
    auto params = translate(functionParams);
    for (const auto &p : params) {
        const auto name = p.name;
        const auto type = p.type;
        auto var = ast::makeNewVar(name, type);
        ctx.local_variables[p.name] = var.get();
    }
    std::vector<std::unique_ptr<ast::Node>> body = translate(fd->body, ctx);
    return ast::makeNewFunction(name, std::move(body), std::move(params));
}

[[nodiscard]] static std::unique_ptr<ast::Node>
translate(const st::ExternalDeclaration &node, Ctx &ctx) {
    const auto &nv = node.node;
    if (std::holds_alternative<std::unique_ptr<st::FuncDef>>(nv)) {
        auto funcdef = std::get<std::unique_ptr<st::FuncDef>>(nv).get();
        return translate(funcdef, ctx);
    }
    const auto &decl = std::get<st::Declaration>(nv);
    return translate(decl, ctx);
}

[[nodiscard]] std::vector<std::unique_ptr<ast::Node>>
translate(const st::Program &program) {
    auto ctx = Ctx{
        .counter = 0,
        .local_variables = {},
    };
    std::vector<std::unique_ptr<ast::Node>> nodes;
    for (const auto &decl : program.nodes) {
        auto node = translate(decl, ctx);
        nodes.push_back(std::move(node));
    }
    return nodes;
}
