
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

std::unique_ptr<Node> makeNewReturn(std::unique_ptr<ast::Node> expr) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::Return;
    node->expr = std::move(expr);
    return node;
}

} // namespace ast