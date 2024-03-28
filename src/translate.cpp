#include "translate.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <variant>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

struct Ctx {
    unsigned long counter = 0;

    std::unordered_map<std::string, ast::Node> local_variables;
    std::unordered_map<std::string, ast::FrameParam> params;
};

[[nodiscard]] ast::DataType *
toDataType(const std::vector<st::DeclarationSpecifier> &dss) {
    return new ast::DataType{
        .name = "int",
        .size = 4,
        .pointsTo = nullptr,
    };
}

[[nodiscard]] ast::DataType *toDataType(const st::Declarator &decl,
                                        ast::DataType *pointsTo) {
    auto ptr = decl.pointer;
    if (!ptr)
        return nullptr;
    auto levels = ptr.value().level;
    ast::DataType *result = pointsTo;
    for (size_t i = 0; i < levels; i++) {
        result = new ast::DataType{
            .name = "pointer",
            .size = 8,
            .pointsTo = result,
        };
    }
    return result;
}

// primary
[[nodiscard]] std::unique_ptr<ast::Node>
translate(std::unique_ptr<st::PrimaryExpression> expr, Ctx &ctx) {
    if (expr->type == st::PrimaryExpressionType::INT) {
        return ast::makeConstInt(expr->value);
    }
    throw std::runtime_error(
        "translate(st::PrimaryExpression *expr, Ctx &ctx) not implemented");
}

// expression
[[nodiscard]] std::unique_ptr<ast::Node> translate(st::Expression expr,
        Ctx &ctx) {
    auto e = std::move(expr.expr);
    return std::visit(
    [&ctx](auto &&arg) -> std::unique_ptr<ast::Node> {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T,
                      std::unique_ptr<st::PrimaryExpression>>) {
            return translate(std::move(arg), ctx);
        } else {
            throw std::runtime_error("translate(st::Expression expr, Ctx &ctx)");
        }
    },
    e);
}

// return statement
[[nodiscard]] std::unique_ptr<ast::Node>
translate(const std::unique_ptr<st::ReturnStatement> stmt, Ctx &ctx) {
    auto expr = translate(std::move(stmt->expr), ctx);
    return ast::makeNewReturn(std::move(expr));
}

// expression statement
[[nodiscard]] std::unique_ptr<ast::Node>
translate(const std::unique_ptr<st::ExpressionStatement> stmt, Ctx &ctx) {
    return translate(std::move(stmt->expr), ctx);
}

// declaration
[[nodiscard]] std::unique_ptr<ast::Node>
translate(const st::Declaration &decl) {
    throw std::runtime_error("translate(const st::Declaration &decl, Ctx &ctx)");
}

[[nodiscard]] std::vector<ast::FrameParam>
translate(const st::ParamTypeList &params) {
    std::vector<ast::FrameParam> result;
    for (const auto &p : params.params) {
        const auto name = p.Name();
        const auto declarator = p.declarator;
        const auto finalType = toDataType(p.declarationSpecifiers);
        const auto paramType = toDataType(declarator, finalType);
        const auto fp = ast::FrameParam{
            .name = name,
            .type = *paramType,
        };
        result.push_back(fp);
    }
    return result;
}

[[nodiscard]] static std::unique_ptr<ast::Node>
translate(const st::Declaration &decl, Ctx &ctx) {
    throw std::runtime_error("translate(const st::Declaration &decl, Ctx &ctx)");
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
        } else {
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
            const auto &decl = std::get<st::Declaration>(bi.item);
            auto node = translate(decl, ctx);
            result.push_back(std::move(node));
        }
    }
    return result;
}

[[nodiscard]] static std::unique_ptr<ast::Node>
translate(const std::unique_ptr<st::FuncDef> &fd, Ctx &ctx) {
    const auto name = fd->Name();
    const auto &functionParams = fd->DirectDeclarator().params;
    auto params = translate(functionParams);
    for (const auto &p : params) {
        ctx.params[p.name] = p;
    }
    std::vector<std::unique_ptr<ast::Node>> body = translate(fd->body, ctx);
    return ast::makeNewFunction(name, std::move(body), std::move(params));
}

[[nodiscard]] static std::unique_ptr<ast::Node>
translate(const st::ExternalDeclaration &node, Ctx &ctx) {
    const auto &nv = node.node;
    if (std::holds_alternative<std::unique_ptr<st::FuncDef>>(nv)) {
        const auto &funcdef = std::get<std::unique_ptr<st::FuncDef>>(nv);
        return translate(std::move(funcdef), ctx);
    }
    const auto &decl = std::get<st::Declaration>(nv);
    return translate(decl, ctx);
}

[[nodiscard]] std::vector<std::unique_ptr<ast::Node>>
translate(const st::Program &program) {
    auto ctx = Ctx{
        .counter = 0,
        .local_variables = {},
        .params = {},
    };
    std::vector<std::unique_ptr<ast::Node>> nodes;
    for (const auto &decl : program.nodes) {
        auto node = translate(decl, ctx);
        nodes.push_back(std::move(node));
    }
    return nodes;
}
#pragma clang diagnostic pop