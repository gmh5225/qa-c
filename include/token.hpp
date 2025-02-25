#pragma once

#include <string>

enum TokType {
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_SEMICOLON,
    TOKEN_SLASH,
    TOKEN_STAR,

    TOKEN_AMPERSAND,

    // One or two character tokens.
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_COLON,
    TOKEN_ARROW,

    // Literals.
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,

    // Keywords.
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    // Builtin types
    TOKEN_T_INT,
    TOKEN_T_INT_LIT,
    TOKEN_T_DOUBLE,

    TOKEN_FEOF
};

struct Token {
    TokType type;
    std::string lexeme;
};
