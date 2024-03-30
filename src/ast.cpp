
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
    node->left = std::move(left);
    node->right = std::move(right);
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
    return node;
}

std::unique_ptr<Node> makeNewAddr(std::unique_ptr<ast::Node> expr) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::Addr;
    node->expr = std::move(expr);
    return node;
}

} // namespace ast