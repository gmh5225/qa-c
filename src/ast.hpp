#pragma once
#include <memory>
#include <string>
#include <vector>

namespace ast {
enum class NodeType { ConstInt, Frame, Move, Return, Temp, Seq };

struct DataType {
    std::string name;
    int size;
    DataType *pointsTo;
};

struct FrameParam {
    std::string name;
    DataType type;
};

class Node {
  public:
    NodeType type;

    // CONST INT
    int value;

    // FRAME
    std::string functionName;
    std::vector<std::unique_ptr<ast::Node>> body;
    std::vector<FrameParam> params;

    std::string variableName = "";
    std::string tempName = "";

    // BIN
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;

    // UNARY
    std::unique_ptr<Node> expr;
};

std::unique_ptr<Node> makeConstInt(int value);

std::unique_ptr<Node>
makeNewFunction(std::string functionName,
                std::vector<std::unique_ptr<ast::Node>> body,
                std::vector<FrameParam> params);

std::unique_ptr<Node> makeNewReturn(std::unique_ptr<ast::Node> expr);
inline std::ostream &operator<<(std::ostream &os, const Node &node);
inline std::ostream &DebugFrame(std::ostream &os, const Node &node) {
    os << "Frame(name=" << node.functionName << ", params=[";
    for (const auto &param : node.params) {
        os << param.name << ", ";
    }
    os << "], body=[";
    for (const auto &n : node.body) {
        os << *n << ", ";
    }
    os << "])";
    return os;
}

inline std::ostream &operator<<(std::ostream &os, const Node &node) {
    switch (node.type) {
    case NodeType::ConstInt:
        os << "ConstInt(" << node.value << ")";
        break;
    case NodeType::Frame:
        DebugFrame(os, node);
        break;
    case NodeType::Move:
        os << "Move(" << *node.left << ", " << *node.right << ")";
        break;
    case NodeType::Return:
        os << "Return(" << *node.expr << ")";
        break;
    case NodeType::Temp:
        os << "Temp(" << node.tempName << ")";
        break;
    case NodeType::Seq:
        os << "Seq(";
        for (const auto &n : node.body) {
            os << *n << ", ";
        }
        os << ")";
        break;
    }
    return os;
}
} // namespace ast
