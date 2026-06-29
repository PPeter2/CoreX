#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "corex/lexer/Token.h"
#include "corex/lexer/TokenType.h"

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();

private:
    std::string source;
    size_t pos;
    int line;
    int column;
    std::vector<Token> tokens;

    static const std::unordered_map<std::string, TokenType> keywords;

    bool isAtEnd() const;
    char peek() const;
    char peekNext() const;
    char advance();
    bool match(char expected);

    void addToken(TokenType type, const std::string& lexeme, int startLine, int startColumn);

    void skipWhitespaceAndComments();
    void scanToken();
    void scanIdentifierOrKeyword();
    void scanNumber();
    void scanString();
    void scanChar();
    void scanAttribute();

    [[noreturn]] void error(const std::string& message, int errLine, int errColumn);
};
