#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <variant>
#include <vector>

#include "ast.hpp"
#include "st.hpp"
namespace ast {
struct Ctx {
    unsigned long counter = 0;
    bool __lvalueContext = false;

    std::unordered_map<std::string, VariableAstNode*> local_variables;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

    void set_lvalueContext(std::string why, bool value) {
        // std::cout << "set_lvalueContext: " << why << " " << value <<
        // std::endl;
        __lvalueContext = value;
    }
#pragma clang diagnostic pop
};

[[nodiscard]] auto translate(const st::Expression& expr, Ctx& ctx)
    -> std::unique_ptr<AstNode>;

[[nodiscard]] auto translate(st::CompoundStatement& stmts, Ctx& ctx)
    -> std::vector<std::unique_ptr<AstNode>>;

[[nodiscard]] auto translate(const std::unique_ptr<st::PrimaryExpression>& expr,
                             Ctx& ctx) -> std::unique_ptr<AstNode>;

[[nodiscard]] auto translate(
    const std::unique_ptr<st::SelectionStatement>& stmt, Ctx& ctx)
    -> std::unique_ptr<AstNode>;

[[nodiscard]] auto translateStatement(st::Statement& stmt, Ctx& ctx)
    -> std::unique_ptr<AstNode>;

[[nodiscard]] auto translate(
    const std::unique_ptr<st::FunctionCallExpression>& expr, Ctx& ctx)
    -> std::unique_ptr<AstNode>;

[[nodiscard]] auto translate(const std::unique_ptr<st::ForStatement>& stmt,
                             Ctx& ctx) -> std::unique_ptr<AstNode>;
[[nodiscard]] auto translate(const st::Declaration& decl, Ctx& ctx)
    -> std::unique_ptr<AstNode>;
[[nodiscard]] std::vector<std::unique_ptr<AstNode>> translate(
    const st::Program& program);
}  // namespace ast