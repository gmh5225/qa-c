#include "assem.hpp"
#include "system.hpp"
#include <iostream>
#include <map>
#include <stdexcept>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

namespace as {
struct Ctx {
    int offset;
    int counter;
    std::map<std::string, int> variableUsage;
    std::map<std::string, ast::DataType> variableType;
    std::map<std::string, StackLocation> variableLocation;

    Temp newTemp(int size) {
        assert(size == 4 || size == 8);
        return Temp{counter++, size};
    }

    StackLocation newVar(ast::Node *node) {
        std::cout << "newVar: " << node->variableName << std::endl;
        offset += node->variableType.size;
        assert(node->variableType.size == 4 || node->variableType.size == 8);
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
};

std::ostream &operator<<(std::ostream &os, const Location &loc) {
    if (std::holds_alternative<Temp>(loc)) {
        os << "t" << std::get<Temp>(loc).id;
    } else if (std::holds_alternative<HardcodedRegister>(loc)) {
        os << std::get<HardcodedRegister>(loc).name;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const Operation &ins) {
    switch (ins.op) {
    case OpCode::Mov:
        os << "mov ";
        break;
    case OpCode::LoadI:
        os << "loadI ";
        break;
    default:
        throw std::runtime_error("Unknown OpCode");
    }
    if (auto dst = std::get_if<Temp>(&ins.dest)) {
        os << *dst << " ";
    } else if (auto dst = std::get_if<HardcodedRegister>(&ins.dest)) {
        os << *dst << " ";
    } else if (auto dst = std::get_if<StackLocation>(&ins.dest)) {
        os << "rbp - " << dst->offset << " ";
    }
    if (auto src = std::get_if<Temp>(&ins.src)) {
        os << *src << " ";
    } else if (auto src = std::get_if<HardcodedRegister>(&ins.src)) {
        os << *src << " ";
    } else if (auto src = std::get_if<StackLocation>(&ins.src)) {
        os << "rbp - " << src->offset << " ";
    }
    if (ins.value.has_value()) {
        os << "value: " << ins.value.value();
    }
    return os;
}

[[nodiscard]] std::string toAsm(const Location &loc) {
    if (std::holds_alternative<Temp>(loc)) {
        throw std::runtime_error("Temp not implemented");
    } else if (std::holds_alternative<HardcodedRegister>(loc)) {
        return std::get<HardcodedRegister>(loc).toString();
    } else if (std::holds_alternative<StackLocation>(loc)) {
        auto size = std::get<StackLocation>(loc).size;
        if (size == 4) {
            return "dword [rbp - " +
                   std::to_string(std::get<StackLocation>(loc).offset) + "]";
        } else if (size == 8) {
            return "qword [rbp - " +
                   std::to_string(std::get<StackLocation>(loc).offset) + "]";
        }
    }
    throw std::runtime_error("toAsm not implemented");
}

Operation LoadI(Location dst, int value) {
    return Operation{.op = OpCode::LoadI,
                     .dest = dst,
                     .src = std::monostate{},
                     .value = value};
}

[[nodiscard]] int SizeOf(const Location loc) {
    if (std::holds_alternative<Temp>(loc)) {
        auto res = std::get<Temp>(loc).size;
        assert(res == 4 || res == 8);
        return res;
    } else if (std::holds_alternative<HardcodedRegister>(loc)) {
        auto res = std::get<HardcodedRegister>(loc).size;
        assert(res == 4 || res == 8);
        return res;
    } else {
        auto res = std::get<StackLocation>(loc).size;
        assert(res == 4 || res == 8);
        return res;
    }
    throw std::runtime_error("SizeOf not implemented");
}

void _AllocParam(std::vector<Instruction> &ins, std::string conventionReg,
                 ast::FrameParam p, Ctx &ctx) {
    assert(p.type.size == 4 || p.type.size == 8);
    // create a variable node for the paramter
    auto var = std::make_unique<ast::Node>();
    var->type = ast::NodeType::Var;
    var->variableType = p.type;
    var->variableName = p.name;
    // allocate the variable
    auto dst = ctx.newVar(var.get());
    auto src = HardcodedReg(conventionReg, p.type.size);
    ins.push_back(Mov(dst, src));
}

Location MunchExpr(std::vector<Instruction> &ins,
                   const std::unique_ptr<ast::Node> &node, Ctx &ctx) {
    switch (node->type) {
    case ast::NodeType::ConstInt: {
        auto temp = ctx.newTemp(4);
        ins.push_back(LoadI(temp, node->value));
        return temp;
    }
    case ast::NodeType::Var: {
        auto loc = ctx.getVar(node.get());
        assert(loc.size == 4 || loc.size == 8);
        auto temp = ctx.newTemp(loc.size);
        ins.push_back(Mov(temp, loc));
        return temp;
    }
    break;
    default:
        break;
    }
    throw std::runtime_error("MunchExpr not implemented");
}

void MunchStmt(std::vector<Instruction> &ins,
               const std::unique_ptr<ast::Node> &node, Ctx &ctx) {
    switch (node->type) {
    case ast::NodeType::Move: {
        auto dst = ctx.newVar(node->left.get());
        auto src = MunchExpr(ins, node->right, ctx);
        ins.push_back(Mov(dst, src));
        return;
    }
    return;
    case ast::NodeType::Return: {
        std::unique_ptr<ast::Node> expr = std::move(node->expr);
        auto src = MunchExpr(ins, expr, ctx);
        auto dst = HardcodedReg("ax", SizeOf(src));
        ins.push_back(Mov(dst, src));
        return;
    }
    break;
    case ast::NodeType::Jump:
        break;
    default:
        break;
    }
    std::cerr << "type: " << static_cast<int>(node->type) << std::endl;
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
        int idx = 0;
        for (const auto &p : node->params) {
            _AllocParam(instructions, target::general_regs[idx], p, ctx);
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

#pragma clang diagnostic pop