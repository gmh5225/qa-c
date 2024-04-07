#pragma once

#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace st {

class PrimaryExpression;
class AssignmentExpression;
class UnaryExpression;
class AdditiveExpression;
class FunctionCallExpression;

using Expression = std::variant<
                   std::unique_ptr<PrimaryExpression>, std::unique_ptr<AssignmentExpression>,
                   std::unique_ptr<UnaryExpression>, std::unique_ptr<AdditiveExpression>,
                   std::unique_ptr<FunctionCallExpression>>;

inline std::ostream &operator<<(std::ostream &os, const Expression &node);

enum class PrimaryExpressionType { INT, IDEN };

class PrimaryExpression {
  public:
    PrimaryExpression(int value)
        : type(PrimaryExpressionType::INT), value(value) {}
    PrimaryExpression(std::string idenValue)
        : type(PrimaryExpressionType::IDEN), idenValue(idenValue) {}

    std::ostream &print(std::ostream &os) {
        if (type == PrimaryExpressionType::INT) {
            os << "PrimaryExpression(type=INT, value=" << value << ")";
        } else {
            os << "PrimaryExpression(type=IDEN, idenValue=" << idenValue << ")";
        }
        return os;
    }

    PrimaryExpressionType type;
    int value;
    std::string idenValue;
};

enum class AdditiveExpressionType { ADD, SUB, EQ, NEQ };

class AdditiveExpression {
  public:
    AdditiveExpression(Expression lhs, Expression rhs,
                       AdditiveExpressionType type);

    std::ostream &print(std::ostream &os) {
        os << "AdditiveExpression(lhs=";
        os << lhs;
        os << ", rhs=";
        os << rhs;
        os << ")";
        return os;
    }

    Expression lhs;
    Expression rhs;
    AdditiveExpressionType type;
};

class AssignmentExpression {
  public:
    AssignmentExpression(Expression lhs, Expression rhs);

    std::ostream &print(std::ostream &os) {
        os << "AssignmentExpression(lhs=";
        os << lhs;
        os << ", rhs=";
        os << rhs;
        os << ")";
        return os;
    }

    Expression lhs;
    Expression rhs;
};

enum class UnaryExpressionType { DEREF, ADDR };

class UnaryExpression {
  public:
    UnaryExpression(UnaryExpressionType type, Expression p_expr);

    std::ostream &print(std::ostream &os) {
        os << "UnaryExpression(type=";
        if (type == UnaryExpressionType::DEREF) {
            os << "DEREF";
        } else {
            os << "ADDR";
        }
        os << ", expr=";
        os << expr;
        os << ")";
        return os;
    }

    UnaryExpressionType type;
    Expression expr;
};

inline std::ostream &operator<<(std::ostream &os, const Expression &expr) {
    os << "Expression(expr=";
    if (std::holds_alternative<std::unique_ptr<PrimaryExpression>>(expr)) {
        return std::get<std::unique_ptr<PrimaryExpression>>(expr)->print(os);
    }
    if (std::holds_alternative<std::unique_ptr<AssignmentExpression>>(expr)) {
        os << "AssignmentExpression(";
        std::get<std::unique_ptr<AssignmentExpression>>(expr).get()->print(os);
        os << ")";
    }
    if (std::holds_alternative<std::unique_ptr<UnaryExpression>>(expr)) {
        os << "UnaryExpression(";
        std::get<std::unique_ptr<UnaryExpression>>(expr).get()->print(os);
        os << ")";
    }
    return os << ")";
}

class FunctionCallExpression {
  public:
    explicit FunctionCallExpression(std::string name,
                                    std::vector<Expression> args);

    std::string name;
    std::vector<Expression> args;
};

class Initalizer {
  public:
    explicit Initalizer(Expression expr) : expr(std::move(expr)) {}
    Expression expr;
};

inline std::ostream &operator<<(std::ostream &os, const Initalizer &node) {
    os << "Initalizer(expr=" << node.expr << ")";
    return os;
}

class Pointer {
  public:
    size_t level;
};

inline std::ostream &operator<<(std::ostream &os, const Pointer &node) {
    os << "Pointer(level=" << node.level << ")";
    return os;
}

inline std::ostream &operator<<(std::ostream &os,
                                const PrimaryExpression &node) {
    if (node.type == PrimaryExpressionType::INT) {
        os << "PrimaryExpression(type=INT, value=" << node.value << ")";
    } else {
        os << "PrimaryExpression(type=IDEN, idenValue=" << node.idenValue << ")";
    }
    return os;
}

class TypeSpecifier {
  public:
    enum class Type { INT, DOUBLE, IDEN };
    Type type;
    std::string iden;
};

inline std::ostream &operator<<(std::ostream &os, const TypeSpecifier &node) {
    os << "TypeSpecifier(type=" << static_cast<int>(node.type)
       << "iden=" << node.iden << ")";
    return os;
}

class DeclarationSpecifier {
  public:
    TypeSpecifier typespecifier;
};

inline std::ostream &operator<<(std::ostream &os,
                                const DeclarationSpecifier &node) {
    os << node.typespecifier;
    return os;
}

class VariableDirectDeclarator {
  public:
    std::string name;
};

class ParameterDeclaration;

class ParamTypeList {
  public:
    std::vector<ParameterDeclaration> params;
    bool va_args;
};

class FunctionDirectDeclarator {
  public:
    VariableDirectDeclarator declarator;
    ParamTypeList params;
};

enum class DeclaratorKind { VARIABLE, FUNCTION };

class DirectDeclarator {
  public:
    DeclaratorKind kind;
    std::variant<VariableDirectDeclarator, FunctionDirectDeclarator> declarator;

    std::string VariableIden() const {
        if (kind == DeclaratorKind::VARIABLE) {
            return std::get<VariableDirectDeclarator>(declarator).name;
        }
        throw std::runtime_error("Not a variable");
    }
};

inline std::ostream &operator<<(std::ostream &os,
                                const DirectDeclarator &node) {
    if (std::holds_alternative<VariableDirectDeclarator>(node.declarator)) {
        os << "DirectDeclarator(kind=VARIABLE, declarator="
           << std::get<VariableDirectDeclarator>(node.declarator).name << ")";
        return os;
    }
    os << "DirectDeclarator(kind=FUNCTION, declarator="
       << std::get<FunctionDirectDeclarator>(node.declarator).declarator.name
       << ")";
    return os;
}

class Declarator {
  public:
    std::optional<Pointer> pointer;
    DirectDeclarator directDeclarator;
};

class ParameterDeclaration {
  public:
    std::string Name() const {
        if (declarator.directDeclarator.kind == DeclaratorKind::VARIABLE) {
            return std::get<VariableDirectDeclarator>(
                       declarator.directDeclarator.declarator)
                   .name;
        }
        throw std::runtime_error("Not a variable");
    }

    std::vector<DeclarationSpecifier> declarationSpecifiers;
    Declarator declarator;

    Declarator Declarator() const {
        return declarator;
    }
};

inline std::ostream &operator<<(std::ostream &os, const Declarator &node) {
    if (node.pointer) {
        os << "Declarator(pointer=" << *node.pointer
           << ", directDeclarator=" << node.directDeclarator << ")";
        return os;
    }
    os << "Declarator(pointer=nullptr"
       << ", directDeclarator=" << node.directDeclarator << ")";
    return os;
}

class InitDeclarator {
  public:
    Declarator declarator;
    std::optional<Initalizer> initializer;
};

inline std::ostream &operator<<(std::ostream &os, const InitDeclarator &node) {
    os << "InitDeclarator(declarator=" << node.declarator << ", initializer=";
    if (node.initializer) {
        os << *node.initializer;
    } else {
        os << "nullptr";
    }
    os << ")";
    return os;
}

class Declaration {
  public:
    std::vector<DeclarationSpecifier> declarationSpecifiers;
    std::optional<InitDeclarator> initDeclarator;

    std::optional<Declarator> Declarator() const {
        if (initDeclarator) {
            return initDeclarator->declarator;
        }
        return std::nullopt;
    }
};

inline std::ostream &operator<<(std::ostream &os, const Declaration &node) {
    os << "Declaration(declarationSpecifiers=[";
    for (auto &v : node.declarationSpecifiers) {
        os << v << ",";
    }
    os << "], initDeclarator=";
    if (node.initDeclarator) {
        os << *node.initDeclarator;
    } else {
        os << "nullptr";
    }
    os << ")";
    return os;
}

class ExpressionStatement {
  public:
    explicit ExpressionStatement(Expression expr) : expr(std::move(expr)) {}

    std::ostream &print(std::ostream &os) const {
        os << "ExpressionStatement(expr=";
        os << expr;
        os << ")";
        return os;
    }

    Expression expr;
};

struct CompoundStatement;

class SelectionStatement {
  public:
    explicit SelectionStatement(Expression cond,
                                std::unique_ptr<CompoundStatement> then,
                                std::unique_ptr<CompoundStatement> else_);

    std::ostream &print(std::ostream &os) const {
        os << "SelectionStatement(cond=";
        os << cond;
        os << ", then=";
        os << then;
        os << ", else=";
        os << else_;
        os << ")";
        return os;
    }

    Expression cond;
    std::unique_ptr<CompoundStatement> then;
    std::unique_ptr<CompoundStatement> else_;
};

class ReturnStatement {
  public:
    explicit ReturnStatement(Expression expr) : expr(std::move(expr)) {}

    std::ostream &print(std::ostream &os) const {
        os << "ReturnStatement(expr=";
        os << expr;
        os << ")";
        return os;
    }
    Expression expr;
};

class Statement {
  public:
    explicit Statement(std::variant<std::unique_ptr<ExpressionStatement>,
                       std::unique_ptr<ReturnStatement>,
                       std::unique_ptr<SelectionStatement>>
                       stmt);

    explicit Statement(std::unique_ptr<ExpressionStatement> node);

    explicit Statement(std::unique_ptr<ReturnStatement> node);

    explicit Statement(std::unique_ptr<SelectionStatement> node);

    std::variant<std::unique_ptr<ExpressionStatement>,
        std::unique_ptr<ReturnStatement>,
        std::unique_ptr<SelectionStatement>>
        stmt;
};

inline std::ostream &operator<<(std::ostream &os, const Statement &node) {
    if (std::holds_alternative<std::unique_ptr<ExpressionStatement>>(node.stmt)) {
        os << "Statement(ExpressionStatement(";
        return std::get<std::unique_ptr<ExpressionStatement>>(node.stmt)->print(os)
               << "))";
    }
    if (std::holds_alternative<std::unique_ptr<SelectionStatement>>(node.stmt)) {
        os << "Statement(SelectionStatement(";
        return std::get<std::unique_ptr<SelectionStatement>>(node.stmt)->print(os)
               << "))";
    }
    os << "Statement(ReturnStatement(";
    return std::get<std::unique_ptr<ReturnStatement>>(node.stmt)->print(os)
           << "))";
}

class BlockItem {
  public:
    explicit BlockItem(std::variant<Declaration, Statement> item);

    explicit BlockItem(Declaration item);
    explicit BlockItem(Statement item);

    std::variant<Declaration, Statement> item;
};

struct CompoundStatement {
    std::vector<BlockItem> items;
};

inline std::ostream &operator<<(std::ostream &os,
                                const CompoundStatement &node) {
    os << "CompoundStatement(items=[";
    for (auto &v : node.items) {
        if (std::holds_alternative<Declaration>(v.item)) {
            os << std::get<Declaration>(v.item) << std::endl;
        } else {
            const auto &stmt = std::get<Statement>(v.item);
            os << stmt << std::endl;
        }
        os << "\n";
    }
    os << "])";
    return os;
}

class FuncDef {
  public:
    FuncDef(std::vector<DeclarationSpecifier> declarationSpecifiers,
            Declarator declarator, CompoundStatement body)
        : declarationSpecifiers(std::move(declarationSpecifiers)),
          declarator(std::move(declarator)), body(std::move(body)) {}

    std::string Name() const {
        if (declarator.directDeclarator.kind == DeclaratorKind::FUNCTION) {
            return std::get<FunctionDirectDeclarator>(
                       declarator.directDeclarator.declarator)
                   .declarator.name;
        }
        throw std::runtime_error("Not a function");
    }

    FunctionDirectDeclarator DirectDeclarator() const {
        assert(declarator.directDeclarator.kind == DeclaratorKind::FUNCTION);
        return std::get<FunctionDirectDeclarator>(
                   declarator.directDeclarator.declarator);
    }

    std::vector<DeclarationSpecifier> declarationSpecifiers;
    Declarator declarator;
    CompoundStatement body;
};

struct ExternalDeclaration {
    explicit ExternalDeclaration(
        std::variant<Declaration, std::unique_ptr<FuncDef>> node)
        : node(std::move(node)) {}

    explicit ExternalDeclaration(Declaration node)
        : node(std::variant<Declaration, std::unique_ptr<FuncDef>>(
                   std::move(node))) {}

    explicit ExternalDeclaration(std::unique_ptr<FuncDef> node)
        : node(std::variant<Declaration, std::unique_ptr<FuncDef>>(
                   std::move(node))) {}

  public:
    std::variant<Declaration, std::unique_ptr<FuncDef>> node;
};

inline std::ostream &operator<<(std::ostream &os,
                                const ExternalDeclaration &node) {
    if (std::holds_alternative<Declaration>(node.node)) {
        const auto &decl = std::get<Declaration>(node.node);
        os << "ExternalDeclaration(Declaration(declarationSpecifiers=[";
        for (auto &v : decl.declarationSpecifiers) {
            os << v << ",";
        }
        os << "], initDeclarator=";
        if (decl.initDeclarator) {
            os << *decl.initDeclarator;
        } else {
            os << "nullptr";
        }
    } else {
        const auto &funcdef = std::get<std::unique_ptr<FuncDef>>(node.node);
        os << "ExternalDeclaration(FuncDef(declarationSpecifiers=[";
        for (auto &v : funcdef->declarationSpecifiers) {
            os << v << ",";
        }
        os << "]\n, declarator=" << funcdef->declarator
           << "\n, body=" << funcdef->body;
    }
    return os;
}

class Program {
  public:
    explicit Program(std::vector<ExternalDeclaration> nodes)
        : nodes(std::move(nodes)) {}
    std::vector<ExternalDeclaration> nodes;
};

} // namespace st