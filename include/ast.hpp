#pragma once
#include <memory>
#include <string>
#include <vector>

#include "st.hpp"

namespace ast {

enum class BinOpKind { Add, Sub, Eq, Gt, Lt, Neq };

[[nodiscard]] auto is_arithmetic(BinOpKind kind) -> bool;
[[nodiscard]] auto is_comparison(BinOpKind kind) -> bool;

enum class SelectionKind { If };
struct DataType {
    std::string name;
    int size = 0;
    DataType* pointsTo = nullptr;

    DataType() : name(""), size(0), pointsTo(nullptr) {}

    DataType(std::string name, int size, DataType* pointsTo)
        : name(name), size(size), pointsTo(pointsTo) {}

    DataType(const DataType& other)
        : name(other.name), size(other.size), pointsTo(nullptr) {
        if (other.pointsTo != nullptr) {
            pointsTo = new DataType(*other.pointsTo);
        }
    }

    DataType& operator=(const DataType& other) {
        if (this != &other) {
            name = other.name;
            size = other.size;
            delete pointsTo;
            pointsTo = nullptr;
            if (other.pointsTo != nullptr) {
                pointsTo = new DataType(*other.pointsTo);
            }
        }
        return *this;
    }

    DataType FinalPointsTo() const {
        if (pointsTo == nullptr) {
            return *this;
        }
        return pointsTo->FinalPointsTo();
    }

    std::string ToString() const {
        if (pointsTo == nullptr) {
            return name;
        }
        return name + " -> " + pointsTo->ToString();
    }
};

struct FrameParam {
    std::string name;
    DataType type;
};

class AstNode {
   public:
    virtual ~AstNode() = default;
    virtual std::string toString() const { return "AstNode"; }
    virtual const ast::BinOpKind* get_bin_op() const { return nullptr; }
};

class ConstIntAstNode : public AstNode {
   public:
    int value;

    explicit ConstIntAstNode(int value) : value(value) {}
};

class ReturnAstNode : public AstNode {
   public:
    std::unique_ptr<AstNode> expr;

    explicit ReturnAstNode(std::unique_ptr<AstNode> expr)
        : expr(std::move(expr)) {}
};

class FrameAstNode : public AstNode {
   public:
    std::string functionName;
    std::vector<std::unique_ptr<AstNode>> body;
    std::vector<FrameParam> params;

    FrameAstNode(std::string functionName,
                 std::vector<std::unique_ptr<AstNode>> body,
                 std::vector<FrameParam> params)
        : functionName(std::move(functionName)),
          body(std::move(body)),
          params(std::move(params)) {}
};

class MoveAstNode : public AstNode {
   public:
    std::unique_ptr<AstNode> lhs;
    std::unique_ptr<AstNode> rhs;

    MoveAstNode(std::unique_ptr<AstNode> lhs, std::unique_ptr<AstNode> rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
};

class BinaryOpAstNode : public AstNode {
   public:
    std::unique_ptr<AstNode> lhs;
    std::unique_ptr<AstNode> rhs;
    BinOpKind kind;

    BinaryOpAstNode(std::unique_ptr<AstNode> lhs, std::unique_ptr<AstNode> rhs,
                    BinOpKind kind)
        : lhs(std::move(lhs)), rhs(std::move(rhs)), kind(kind) {}

    const BinOpKind* get_bin_op() const override { return &kind; }
};

class DerefReadAstNode : public AstNode {
   public:
    std::unique_ptr<AstNode> expr;

    explicit DerefReadAstNode(std::unique_ptr<AstNode> expr)
        : expr(std::move(expr)) {}
};

class DerefWriteAstNode : public AstNode {
   public:
    std::unique_ptr<AstNode> expr;

    explicit DerefWriteAstNode(std::unique_ptr<AstNode> expr)
        : expr(std::move(expr)) {}
};

class AddrAstNode : public AstNode {
   public:
    std::unique_ptr<AstNode> expr;

    explicit AddrAstNode(std::unique_ptr<AstNode> expr)
        : expr(std::move(expr)) {}
};

class JumpAstNode : public AstNode {
   public:
    std::string jumpToLabelValue;

    explicit JumpAstNode(std::string jumpToLabelValue)
        : jumpToLabelValue(std::move(jumpToLabelValue)) {}
};

class VariableAstNode : public AstNode {
   public:
    std::string variableName;
    ast::DataType variableType;

    explicit VariableAstNode(std::string variableName,
                             ast::DataType variableType)
        : variableName(std::move(variableName)), variableType(variableType) {}
};

class IfNode : public AstNode {
   public:
    std::unique_ptr<AstNode> condition;
    std::vector<std::unique_ptr<AstNode>> then;
    std::optional<std::vector<std::unique_ptr<AstNode>>> else_;

    IfNode(std::unique_ptr<AstNode> condition,
           std::vector<std::unique_ptr<AstNode>> then,
           std::vector<std::unique_ptr<AstNode>> else_)
        : condition(std::move(condition)),
          then(std::move(then)),
          else_(std::move(else_)) {}

    IfNode(std::unique_ptr<AstNode> condition,
           std::vector<std::unique_ptr<AstNode>> then,
           std::optional<std::vector<std::unique_ptr<AstNode>>> else_)
        : condition(std::move(condition)),
          then(std::move(then)),
          else_(std::move(else_)) {}
};

class FunctionCallAstNode : public AstNode {
   public:
    std::string callName;
    std::vector<std::unique_ptr<AstNode>> callArgs;
    ast::DataType returnType;

    FunctionCallAstNode(std::string callName,
                        std::vector<std::unique_ptr<AstNode>> callArgs,
                        ast::DataType returnType)
        : callName(std::move(callName)),
          callArgs(std::move(callArgs)),
          returnType(returnType) {}
};

class ForLoopAstNode : public AstNode {
   public:
    std::unique_ptr<AstNode> forInit;
    std::optional<std::unique_ptr<AstNode>> forCondition;
    std::optional<std::unique_ptr<AstNode>> forUpdate;
    std::vector<std::unique_ptr<AstNode>> forBody;

    ForLoopAstNode(std::unique_ptr<AstNode> forInit,
                   std::optional<std::unique_ptr<AstNode>> forCondition,
                   std::optional<std::unique_ptr<AstNode>> forUpdate,
                   std::vector<std::unique_ptr<AstNode>> forBody)
        : forInit(std::move(forInit)),
          forCondition(std::move(forCondition)),
          forUpdate(std::move(forUpdate)),
          forBody(std::move(forBody)) {}
};

}  // namespace ast
