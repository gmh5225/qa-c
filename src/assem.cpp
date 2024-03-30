
#include <iostream>
#include <map>
#include <stdexcept>

#include "assem.hpp"
#include "location.hpp"
#include "qa_x86.hpp"

namespace as {

struct Ctx {
    int offset;
    int counter;
    std::map<std::string, int> variableUsage;
    std::map<std::string, ast::DataType> variableType;
    std::map<std::string, StackLocation> variableLocation;

    Temp newTemp(int size) {
        return Temp{counter++, size};
    }

    StackLocation newVar(ast::Node *node) {
        offset += node->variableType.size;
        variableLocation[node->variableName] =
            StackLocation{offset, node->variableType.size};
        variableType[node->variableName] = node->variableType;
        return StackLocation{offset, node->variableType.size};
    }

    StackLocation getVar(ast::Node *node) {
        if (variableLocation.find(node->variableName) == variableLocation.end()) {
            throw std::runtime_error("Variable not found");
        }
        return variableLocation[node->variableName];
    }

    StackLocation __GetOrDefineVar(ast::Node *node) {
        try {
            return getVar(node);
        } catch (const std::exception &e) {
            return newVar(node);
        }
    }
};

Location MunchExpr(std::vector<Instruction> &ins,
                   const std::unique_ptr<ast::Node> &node, Ctx &ctx);

Location MunchExpr(std::vector<Instruction> &ins,
                   const std::unique_ptr<ast::Node> &node, Ctx &ctx) {
    switch (node->type) {
    case ast::NodeType::ConstInt: {
        auto temp = ctx.newTemp(4);
        ins.push_back(LoadI(temp, node->value));
        return temp;
    }
    case ast::NodeType::Var: {
        return ctx.__GetOrDefineVar(node.get());
    }
    case ast::NodeType::MemRead: {
        auto autoAddressOrIntermediate = MunchExpr(ins, node->expr, ctx);
        // if we got a variable on our MunchExpr call, it is an address
        if (std::holds_alternative<StackLocation>(autoAddressOrIntermediate)) {
            // move to a temp return temp
            auto temp = ctx.newTemp(8);
            ins.push_back(Mov(temp, autoAddressOrIntermediate));
            return temp;
        }
        // didn't return a stack location, instead returned the intermediate
        // representing the load result
        auto tempToHoldAddress = ctx.newTemp(8);
        ins.push_back(Load(tempToHoldAddress, autoAddressOrIntermediate));
        return tempToHoldAddress;
    }
    case ast::NodeType::MemWrite: {
        auto addressOrIntermediate = MunchExpr(ins, node->expr, ctx);
        auto temp1 = ctx.newTemp(8);
        if (node->expr->type == ast::NodeType::MemWrite) {
            // it is a load
            ins.push_back(Load(temp1, addressOrIntermediate));
        } else {
            ins.push_back(Mov(temp1, addressOrIntermediate));
        }
        return temp1;
    }
    case ast::NodeType::Addr: {
        return MunchExpr(ins, node->expr, ctx);
    }
    default:
        break;
    }
    throw std::runtime_error("MunchExpr not implemented");
}

void MunchStmt(std::vector<Instruction> &ins,
               const std::unique_ptr<ast::Node> &node, Ctx &ctx) {
    switch (node->type) {
    case ast::NodeType::Move: {
        switch (node->left->type) {
        case ast::NodeType::Var: {
            auto dst = ctx.__GetOrDefineVar(node->left.get());
            switch (node->right->type) {
            case ast::NodeType::ConstInt: {
                ins.push_back(LoadI(dst, node->right->value));
                return;
            }
            case ast::NodeType::Addr: {
                auto loc = ctx.getVar(node->right->expr.get());
                auto temp = ctx.newTemp(8);
                ins.push_back(Addr(temp, loc));
                ins.push_back(Mov(dst, temp));
                return;
            }
            case ast::NodeType::MemRead: {
                auto addressToReadFrom = MunchExpr(ins, node->right, ctx);
                auto temp = ctx.newTemp(dst.size);
                ins.push_back(Load(temp, addressToReadFrom));
                ins.push_back(Mov(dst, temp));
                return;
            }
            case ast::NodeType::Var: {
                auto src = ctx.getVar(node->right.get());
                auto tmp = ctx.newTemp(src.size);
                ins.push_back(Mov(tmp, src));
                ins.push_back(Mov(dst, tmp));
                return;
            }
            default:
                throw std::runtime_error("Not a valid rhs for Move(Var, rhs)");
            }
        }
        case ast::NodeType::MemWrite: {
            switch (node->right->type) {
            case ast::NodeType::ConstInt: {
                auto dst = MunchExpr(ins, node->left, ctx);
                ins.push_back(StoreI(dst, node->right->value));
                return;
            }
            case ast::NodeType::MemRead: {
                auto addressToReadFrom = MunchExpr(ins, node->right, ctx);
                auto dst = MunchExpr(ins, node->left, ctx);
                auto temp = ctx.newTemp(SizeOf(dst));
                ins.push_back(Load(temp, addressToReadFrom));
                ins.push_back(Store(dst, temp));
                return;
            }
            case ast::NodeType::Var: {
                auto src = ctx.getVar(node->right.get());
                auto tmp = ctx.newTemp(src.size);
                ins.push_back(Mov(tmp, src));
                auto dst = MunchExpr(ins, node->left, ctx);
                ins.push_back(Store(dst, tmp));
                return;
            }
            default:
                throw std::runtime_error("Not a valid rhs for Move(Var, rhs)");
            }
        }
        default:
            throw std::runtime_error("Not a valid target for move");
        }
    }
    case ast::NodeType::Return: {
        switch (node->expr->type) {
        case ast::NodeType::ConstInt: {
            auto dest = HardcodedRegister{.reg = target::BaseRegister::AX, .size = 4};
            ins.push_back(LoadI(dest, node->expr->value));
            return;
        }
        case ast::NodeType::Var: {
            auto src = ctx.getVar(node->expr.get());
            auto tmp = ctx.newTemp(src.size);
            ins.push_back(Mov(tmp, src));
            auto dest =
                HardcodedRegister{.reg = target::BaseRegister::AX, .size = src.size};
            ins.push_back(Mov(dest, tmp));
            return;
        }
        case ast::NodeType::MemRead: {
            auto variableName = node->expr->VariableName();
            const auto datatype = ctx.variableType[variableName]  ;
            const auto pointsToSize = datatype.FinalPointsTo().size;
            auto addressToReadFrom = MunchExpr(ins, node->expr, ctx);
            auto temp = ctx.newTemp(pointsToSize);
            ins.push_back(Load(temp, addressToReadFrom));
            auto dest = HardcodedRegister{.reg = target::BaseRegister::AX, .size = pointsToSize};
            ins.push_back(Mov(dest, temp));
            return;
        }
        default:
            throw std::runtime_error("Not a valid return type");
        }
    }
    case ast::NodeType::Jump:
        break;
    default:
        break;
    }
    throw std::runtime_error("MunchStmt not implemented");
}

[[nodiscard]] std::vector<Frame>
As_Instructions(const std::vector<std::unique_ptr<ast::Node>> &nodes) {
    std::vector<Frame> frames;
    for (const auto &node : nodes) {
        auto ctx = Ctx{.offset = 0, .counter = 0};
        if (node->type != ast::NodeType::Frame) {
            continue;
        }
        const auto name = node->functionName;
        std::vector<Instruction> instructions;
        unsigned long idx = 0;
        for (const auto &p : node->params) {
            // create a variable node for the paramter
            auto var = ast::makeNewVar(p.name, p.type);
            // create a stack location for the variable
            auto dst = ctx.newVar(var.get());
            const auto param_register = target::param_regs.at(idx);
            auto src = HardcodedRegister{.reg = param_register, .size = p.type.size};
            instructions.push_back(Mov(dst, src));
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
} // namespace as
