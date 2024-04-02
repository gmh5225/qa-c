
#include <map>
#include <stdexcept>

#include "assem.hpp"
#include "location.hpp"
#include "qa_x86.hpp"

namespace qa_ir {

struct Ctx {
    int counter;
    int labelCounter;
    std::map<std::string, int> variableUsage;
    std::map<std::string, const ast::Node *> variables;

    Temp newTemp(int size) {
        assert(size != 0);
        return Temp{counter++, size};
    }

    Value AddVariable(const ast::Node *node) {
        auto name = node->variableName;
        variableUsage[name]++;
        variables[name] = node;
        const auto size = node->variableType.size;
        return Variable{name, variableUsage[name], size};
    }
};

Value GenerateIRForRhs(std::vector<Operation> &ins, const ast::Node *node,
                       Ctx &ctx) {
    switch (node->type) {
    case ast::NodeType::ConstInt: {
        return node->value;
    }
    case ast::NodeType::Var: {
        return ctx.AddVariable(node);
    }
    case ast::NodeType::BinOp: {
        auto lhs = GenerateIRForRhs(ins, node->lhs.get(), ctx);
        auto rhs = GenerateIRForRhs(ins, node->rhs.get(), ctx);
        auto dst = ctx.newTemp(SizeOf(lhs));
        if (node->binOpKind == ast::BinOpKind::Add) {
            auto binop_instruction = Add{.dst = dst, .left = lhs, .right = rhs};
            ins.push_back(binop_instruction);
            return dst;
        }
        if (node->binOpKind == ast::BinOpKind::Sub) {
            auto binop_instruction = Sub{.dst = dst, .left = lhs, .right = rhs};
            ins.push_back(binop_instruction);
            return dst;
        }
        return dst;
    }
    default:
        throw std::runtime_error("GenerateIRForRhs not implemented for type: " +
                                 std::to_string(static_cast<int>(node->type)));
    }
}
void GenerateIRForReturnNode(std::vector<Operation> &ins,
                             const std::unique_ptr<ast::Node> &node, Ctx &ctx) {
    auto ret = GenerateIRForRhs(ins, node->expr.get(), ctx);
    auto ret_instruction = Ret{.value = ret};
    ins.push_back(ret_instruction);
}

void GenerateIRForMovNode(std::vector<Operation> &ins,
                          const std::unique_ptr<ast::Node> &node, Ctx &ctx) {
    auto src = GenerateIRForRhs(ins, node->rhs.get(), ctx);
    switch (node->lhs->type) {
    case ast::NodeType::Var: {
        auto dst = ctx.AddVariable(node->lhs.get());
        auto mov_instruction = Mov{.dst = dst, .src = src};
        ins.push_back(mov_instruction);
        return;
    }
    default:
        throw std::runtime_error("GenerateIRForMovNode not implemented for type: " +
                                 std::to_string(static_cast<int>(node->lhs->type)));
    }
}

void MunchStmt(std::vector<Operation> &ins,
               const std::unique_ptr<ast::Node> &node, Ctx &ctx) {
    switch (node->type) {
    case ast::NodeType::Return: {
        GenerateIRForReturnNode(ins, node, ctx);
        return;
    }
    case ast::NodeType::Move: {
        GenerateIRForMovNode(ins, node, ctx);
        return;
    }
    default:
        throw std::runtime_error("MunchStmt not implemented for type: " +
                                 std::to_string(static_cast<int>(node->type)));
    }
}

[[nodiscard]] std::vector<Frame>
Produce_IR(const std::vector<std::unique_ptr<ast::Node>> &nodes) {
    std::vector<Frame> frames;
    for (const auto &node : nodes) {
        auto ctx = Ctx{.counter = 0, .labelCounter = 0};
        if (node->type != ast::NodeType::Frame) {
            continue;
        }
        const auto name = node->functionName;
        std::vector<Operation> instructions;
        unsigned long idx = 0;
        for (const auto &p : node->params) {
            // create a variable node for the paramter
            auto var = ast::makeNewVar(p.name, p.type);
            // create a stack location for the variable
            auto dst = ctx.AddVariable(var.get());
            const auto param_register = target::param_regs.at(idx);
            auto src = HardcodedRegister{.reg = param_register, .size = p.type.size};
            instructions.push_back(MovR{.dst = dst, .src = src});
            idx++;
        }
        auto body = std::move(node->body);
        for (const auto &node : body) {
            MunchStmt(instructions, node, ctx);
        }
        auto frame = Frame{name, instructions};
        frames.push_back(frame);
    }
    return frames;
}
} // namespace qa_ir
