#include "parser.hpp"
#include "st.hpp"
#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>

#include "token.hpp"

[[nodiscard]] st::DirectDeclarator parseDirectDeclartor();
[[nodiscard]] st::Declaration parseDeclaration();
[[nodiscard]] static st::Declarator parseDeclarator();
[[nodiscard]] st::CompoundStatement parseCompoundStatement();
[[nodiscard]] st::Expression parseExpression();

static unsigned long current = 0;
std::vector<Token> g_tokens;

[[nodiscard]] static Token peek() {
    if (current >= g_tokens.size()) {
        return Token{TokType::TOKEN_FEOF, ""};
    }
    return g_tokens[current];
}

[[nodiscard]] Token peekn(size_t n) {
    if (current + n >= g_tokens.size()) {
        return Token{TokType::TOKEN_FEOF, ""};
    }
    return g_tokens[current + n];
}

static Token advance() {
    if (current >= g_tokens.size()) {
        return Token{TokType::TOKEN_FEOF, ""};
    }
    return g_tokens[current++];
}

static Token previous() {
    return g_tokens[current - 1];
}

[[nodiscard]] bool match(TokType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

[[nodiscard]] static bool isAtEnd() {
    return peek().type == TokType::TOKEN_FEOF;
}

[[nodiscard]] bool isTypeSpecifier(Token token) {
    return token.type == TokType::TOKEN_T_INT;
}

void consume(TokType typ) {
    if (match(typ) == false) {
        throw std::runtime_error("Expected token of type " +
                                 std::to_string(static_cast<int>(typ)) + " found " +
                                 std::to_string(static_cast<int>(peek().type)));
    }
}

[[nodiscard]] bool isFuncBegin() {
    return isTypeSpecifier(peek()) &&
           peekn(1).type == TokType::TOKEN_IDENTIFIER &&
           peekn(2).type == TokType::TOKEN_LEFT_PAREN;
}

[[nodiscard]] std::vector<st::DeclarationSpecifier> parseDeclarationSpecs() {
    std::vector<st::DeclarationSpecifier> declspecs;
    while (isTypeSpecifier(peek())) {
        if (peek().type == TokType::TOKEN_T_INT) {
            declspecs.push_back(st::DeclarationSpecifier{
                st::TypeSpecifier{.type = st::TypeSpecifier::Type::INT}});
        } else {
            const auto ts = st::TypeSpecifier{.type = st::TypeSpecifier::Type::IDEN,
                                              .iden = peek().lexeme};
            const auto ds = st::DeclarationSpecifier{.typespecifier = ts};
            declspecs.push_back(ds);
        }
        advance();
    }
    return declspecs;
}

[[nodiscard]] std::optional<st::Pointer> parsePointer() {
    size_t count = 0;
    while (match(TokType::TOKEN_STAR)) {
        count++;
    }
    if (count == 0) {
        return std::nullopt;
    }
    return st::Pointer{.level = count};
}

[[nodiscard]] std::string parseIdentifier() {
    const auto tk = peek();
    if (tk.type == TokType::TOKEN_IDENTIFIER) {
        advance();
        return tk.lexeme;
    }
    std::string msg = "expected identifier, found " + tk.lexeme;
    throw std::runtime_error(msg);
}

[[nodiscard]] st::ParamTypeList parseParamTypeList() {
    std::vector<st::ParameterDeclaration> params;
    while (!match(TokType::TOKEN_RIGHT_PAREN)) {
        const auto declspecs = parseDeclarationSpecs();
        const auto decl = parseDeclarator();
        params.push_back(st::ParameterDeclaration{
            .declarationSpecifiers = declspecs, .declarator = decl});
        if (match(TokType::TOKEN_COMMA) == false) {
            break;
        }
    }
    if (match(TokType::TOKEN_RIGHT_PAREN)) {
    }
    return st::ParamTypeList{.params = params, .va_args = false};
}

[[nodiscard]] st::DirectDeclarator parseDirectDeclartor() {
    auto iden = parseIdentifier();
    if (match(TokType::TOKEN_LEFT_PAREN)) {
        auto paramList = parseParamTypeList();
        auto fd = st::FunctionDirectDeclarator{
            .declarator = st::VariableDirectDeclarator{.name = iden},
            .params = paramList};
        return st::DirectDeclarator{.kind = st::DeclaratorKind::FUNCTION,
                                    .declarator = fd};
    }
    auto vd = st::VariableDirectDeclarator{.name = iden};
    return st::DirectDeclarator{.kind = st::DeclaratorKind::VARIABLE,
                                .declarator = vd};
}

[[nodiscard]] static st::Declarator parseDeclarator() {
    const auto ptr = parsePointer();
    const auto dd = parseDirectDeclartor();
    return st::Declarator{.pointer = ptr, .directDeclarator = dd};
}

[[nodiscard]] bool isStmtBegin(Token t) {
    return t.type == TokType::TOKEN_RETURN ||
           t.type == TokType::TOKEN_IDENTIFIER || t.type == TokType::TOKEN_STAR ||
           t.type == TokType::TOKEN_IF;
}

[[nodiscard]] std::unique_ptr<st::PrimaryExpression> parsePrimaryExpression() {
    if (peek().type == TokType::TOKEN_IDENTIFIER) {
        const auto lexeme = peek().lexeme;
        advance();
        return std::make_unique<st::PrimaryExpression>(lexeme);
    }
    if (peek().type == TokType::TOKEN_NUMBER) {
        const auto lexeme = peek().lexeme;
        advance();
        return std::make_unique<st::PrimaryExpression>(std::stoi(lexeme));
    }
    throw std::runtime_error("Expected primary expression found " +
                             peek().lexeme);
}

[[nodiscard]] bool __EqualsSignLookahead() {
    unsigned long i = current;
    while (i < g_tokens.size()) {
        if (g_tokens[i].type == TokType::TOKEN_EQUAL) {
            return true;
        }
        if (g_tokens[i].type == TokType::TOKEN_SEMICOLON) {
            return false;
        }
        if (g_tokens[i].type == TokType::TOKEN_LEFT_BRACE) {
            return false;
        }
        if (g_tokens[i].type == TokType::TOKEN_RIGHT_BRACE) {
            return false;
        }
        if (g_tokens[i].type == TokType::TOKEN_LEFT_PAREN) {
            return false;
        }
        if (g_tokens[i].type == TokType::TOKEN_RIGHT_PAREN) {
            return false;
        }
        i++;
    }
    return false;
}

[[nodiscard]] st::Expression parsePostfixExpression() {
    auto primary = parsePrimaryExpression();
    if (match(TokType::TOKEN_LEFT_PAREN)) {
        std::vector<st::Expression> args;
        while (!match(TokType::TOKEN_RIGHT_PAREN)) {
            auto expr = parseExpression();
            args.push_back(std::move(expr));
            if (match(TokType::TOKEN_COMMA) == false) {
                break;
            }
        }
        if (match(TokType::TOKEN_RIGHT_PAREN)) {
        }
        const auto name = primary.get()->idenValue;
        return std::make_unique<st::FunctionCallExpression>(name, std::move(args));
    }
    return primary;
}

[[nodiscard]] st::Expression parseUnaryExpression() {
    if (match(TokType::TOKEN_STAR)) {
        auto expr = parseUnaryExpression();
        return std::make_unique<st::UnaryExpression>(st::UnaryExpressionType::DEREF,
                std::move(expr));
    }
    if (match(TokType::TOKEN_AMPERSAND)) {
        auto expr = parseUnaryExpression();
        return std::make_unique<st::UnaryExpression>(st::UnaryExpressionType::ADDR,
                std::move(expr));
    }
    return parsePostfixExpression();
}

[[nodiscard]] st::Expression parseAdditiveExpression() {
    auto lhs = parseUnaryExpression();
    while (match(TokType::TOKEN_PLUS) || match(TokType::TOKEN_MINUS)) {
        auto op_token = previous();
        auto op = st::AdditiveExpressionType::ADD;
        if (op_token.type == TokType::TOKEN_MINUS) {
            op = st::AdditiveExpressionType::SUB;
        }
        auto rhs = parseUnaryExpression();
        lhs = std::make_unique<st::AdditiveExpression>(std::move(lhs),
              std::move(rhs), op);
    }
    return lhs;
}

[[nodiscard]] st::Expression parseRelationalExpression() {
    auto lhs = parseAdditiveExpression();
    if (match(TOKEN_GREATER)) {
        auto op_token = previous();
        auto op = st::AdditiveExpressionType::GT;
        auto rhs = parseAdditiveExpression();
        return std::make_unique<st::AdditiveExpression>(std::move(lhs),
                std::move(rhs), op);
    }
    return lhs;
}

[[nodiscard]] st::Expression parseEqualityExpression() {
    auto lhs = parseRelationalExpression();
    if (match(TokType::TOKEN_EQUAL_EQUAL) || match(TokType::TOKEN_BANG_EQUAL)) {
        auto op_token = previous();
        auto op = st::AdditiveExpressionType::EQ;
        if (op_token.type == TokType::TOKEN_BANG_EQUAL) {
            op = st::AdditiveExpressionType::NEQ;
        }
        auto rhs = parseRelationalExpression();
        return std::make_unique<st::AdditiveExpression>(std::move(lhs),
                std::move(rhs), op);
    }
    return lhs;
}

[[nodiscard]] st::Expression parseAssignmentExpression() {
    if (__EqualsSignLookahead() == false) {
        return parseEqualityExpression();
    }
    auto lhs = parseEqualityExpression();
    consume(TokType::TOKEN_EQUAL);
    auto rhs = parseEqualityExpression();
    return std::make_unique<st::AssignmentExpression>(std::move(lhs),
            std::move(rhs));
}

[[nodiscard]] st::Expression parseExpression() {
    return parseAssignmentExpression();
}

[[nodiscard]] std::unique_ptr<st::ReturnStatement> parseReturnStatement() {
    auto expr = parseExpression();
    consume(TokType::TOKEN_SEMICOLON);
    return std::make_unique<st::ReturnStatement>(std::move(expr));
}

[[nodiscard]] std::unique_ptr<st::ExpressionStatement>
parseExpressionStatement() {
    auto expr = parseExpression();
    consume(TokType::TOKEN_SEMICOLON);
    return std::make_unique<st::ExpressionStatement>(std::move(expr));
}

[[nodiscard]] st::Statement parseStatement();

[[nodiscard]] std::unique_ptr<st::SelectionStatement> parseIfStatement() {
    consume(TokType::TOKEN_LEFT_PAREN);
    auto expr = parseExpression();
    consume(TokType::TOKEN_RIGHT_PAREN);
    auto thenStmt = parseCompoundStatement();
    auto thenStmtUnique =
        std::make_unique<st::CompoundStatement>(std::move(thenStmt));
    if (match(TokType::TOKEN_ELSE)) {
        auto elseStmt = parseCompoundStatement();
        auto elseStmtUnique =
            std::make_unique<st::CompoundStatement>(std::move(elseStmt));
        return std::make_unique<st::SelectionStatement>(
                   std::move(expr), std::move(thenStmtUnique), std::move(elseStmtUnique));
    }
    return std::make_unique<st::SelectionStatement>(
               std::move(expr), std::move(thenStmtUnique), nullptr);
}

[[nodiscard]] st::Statement parseStatement() {
    if (match(TokType::TOKEN_RETURN)) {
        auto ret = parseReturnStatement();
        return st::Statement(std::move(ret));
    }
    if (match(TokType::TOKEN_IF)) {
        auto ifStmt = parseIfStatement();
        return st::Statement(std::move(ifStmt));
    }
    auto expr = parseExpressionStatement();
    return st::Statement(std::move(expr));
}

[[nodiscard]] st::BlockItem parseBlockItem() {
    if (isStmtBegin(peek())) {
        auto item = parseStatement();
        return st::BlockItem(std::move(item));
    }
    auto decl = parseDeclaration();
    return st::BlockItem(std::move(decl));
}

[[nodiscard]] st::CompoundStatement parseCompoundStatement() {
    // left
    consume(TokType::TOKEN_LEFT_BRACE);
    std::vector<st::BlockItem> blockItems;
    while (!match(TokType::TOKEN_RIGHT_BRACE)) {
        auto bi = parseBlockItem();
        blockItems.push_back(std::move(bi));
    }
    return st::CompoundStatement{.items = std::move(blockItems)};
}

[[nodiscard]] st::Initalizer parseInitalizer() {
    auto expr = parseExpression();
    return st::Initalizer(std::move(expr));
}

[[nodiscard]] st::InitDeclarator parseInitDeclarator() {
    auto declarator = parseDeclarator();
    if (match(TokType::TOKEN_EQUAL)) {
        auto initializer = parseInitalizer();
        consume(TokType::TOKEN_SEMICOLON);
        return st::InitDeclarator{.declarator = declarator,
                                  .initializer = std::move(initializer)};
    }
    consume(TokType::TOKEN_SEMICOLON);
    return st::InitDeclarator{.declarator = declarator,
                              .initializer = std::nullopt};
}

[[nodiscard]] st::Declaration parseDeclaration() {
    const auto declspecs = parseDeclarationSpecs();
    // hack obvs
    auto initDeclarator = parseInitDeclarator();
    return st::Declaration{.declarationSpecifiers = declspecs,
                           .initDeclarator = std::move(initDeclarator)};
}

[[nodiscard]] std::unique_ptr<st::FuncDef> parseFunctionDefinition() {
    const auto declspecs = parseDeclarationSpecs();
    const auto decl = parseDeclarator();
    st::CompoundStatement body = parseCompoundStatement();
    return std::make_unique<st::FuncDef>(declspecs, decl, std::move(body));
}

[[nodiscard]] std::optional<st::ExternalDeclaration>
parseExternalDeclaration() {
    const auto nxt = peek();
    if (nxt.type == TokType::TOKEN_SEMICOLON) {
        advance();
        return std::nullopt;
    }
    if (isFuncBegin()) {
        auto fd = parseFunctionDefinition();
        return st::ExternalDeclaration(std::move(fd));
    }
    auto decl = parseDeclaration();
    return st::ExternalDeclaration(std::move(decl));
}

[[nodiscard]] st::Program parse(const std::vector<Token> &tokens) {
    g_tokens = tokens;
    std::vector<st::ExternalDeclaration> nodes;
    while (isAtEnd() == false) {
        auto ed = parseExternalDeclaration();
        if (ed.has_value()) {
            auto decl = std::move(ed.value());
            nodes.push_back(std::move(decl));
        }
    }
    return st::Program(std::move(nodes));
}