#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "ast.hpp"

namespace ast {

std::unique_ptr<Node> makeConstInt(int value) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::ConstInt;
    node->value = value;
    return node;
}

std::unique_ptr<Node> makeNewFunction(std::string functionName,
                                      std::vector<std::unique_ptr<ast::Node>> body,
                                      std::vector<FrameParam> params) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::Frame;
    node->functionName = functionName;
    node->body = std::move(body);
    node->params = params;
    return node;
}

std::unique_ptr<Node> makeJump(std::string label) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::Jump;
    node->jumpToLabelValue = label;
    return node;
}

std::unique_ptr<Node> makeNewReturn(std::unique_ptr<ast::Node> expr) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::Return;
    node->expr = std::move(expr);
    return node;
}

std::unique_ptr<Node> makeNewVar(std::string name, DataType type) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::Var;
    node->variableName = name;
    node->variableType = type;
    return node;
}

std::unique_ptr<Node> makeNewMove(std::unique_ptr<ast::Node> left,
                                  std::unique_ptr<ast::Node> right) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::Move;
    node->lhs = std::move(left);
    node->rhs = std::move(right);
    return node;
}

std::unique_ptr<Node> makeNewMemWrite(std::unique_ptr<ast::Node> expr) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::MemWrite;
    node->expr = std::move(expr);
    return node;
}

std::unique_ptr<Node> makeNewMemRead(std::unique_ptr<ast::Node> expr) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::MemRead;
    node->expr = std::move(expr);
    if (node->expr->type == NodeType::Addr) {
        node->type = node->expr->expr->type;
        // hack to copy, assumes that it is a variable. works now for nested stuff
        node->variableName = node->expr->expr->variableName;
    }
    return node;
}

std::unique_ptr<Node> makeNewAddr(std::unique_ptr<ast::Node> expr) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::Addr;
    node->expr = std::move(expr);
    return node;
}

std::unique_ptr<Node> makeNewBinOp(std::unique_ptr<ast::Node> lhs,
                                   std::unique_ptr<ast::Node> rhs,
                                   BinOpKind kind) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::BinOp;
    if (lhs->type == NodeType::ConstInt && rhs->type == NodeType::ConstInt) {
        switch (kind) {
        case BinOpKind::Add:
            return makeConstInt(lhs->value + rhs->value);
        case BinOpKind::Sub:
            return makeConstInt(lhs->value - rhs->value);
        default:
            throw std::runtime_error("Unknown BinOpKind");
        }
    }
    node->lhs = std::move(lhs);
    node->rhs = std::move(rhs);
    node->binOpKind = kind;
    return node;
}

} // namespace ast