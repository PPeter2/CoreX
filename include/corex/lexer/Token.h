#pragma once
#include <string>
#include "corex/lexer/TokenType.h"

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;

    Token(TokenType type, std::string lexeme, int line, int column)
        : type(type), lexeme(std::move(lexeme)), line(line), column(column) {}

    std::string toString() const {
        return "Token(" + tokenTypeToString(type) + ", '" + lexeme + "', " + std::to_string(line) + ":" + std::to_string(column) + ")";
    }
};
