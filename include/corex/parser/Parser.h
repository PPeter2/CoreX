#pragma once
#include <vector>
#include <memory>
#include "corex/lexer/Token.h"
#include "corex/ast/Expr.h"
#include "corex/ast/Type.h"
#include "corex/ast/Stmt.h"
#include "corex/ast/Decl.h"
#include "corex/ast/Program.h"

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Type> parseType();
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Decl> parseDeclaration();
    std::unique_ptr<Program> parseProgram();

private:
    std::vector<Token> tokens;
    size_t pos;

    const Token& peek() const;
    const Token& peekNext() const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token& expect(TokenType type, const std::string& message);
    bool isAtEnd() const;

    [[noreturn]] void error(const std::string& message);

    std::unique_ptr<Expr> parseAssignment();
    std::unique_ptr<Expr> parseLogicalOr();
    std::unique_ptr<Expr> parseLogicalAnd();
    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseRange();
    std::unique_ptr<Expr> parseAdditive();
    std::unique_ptr<Expr> parseMultiplicative();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePostfix();
    std::unique_ptr<Expr> parsePrimary();
    std::vector<std::unique_ptr<Expr>> parseArgumentList();

    bool checkTypeKeyword() const;
    std::string typeKeywordName(TokenType type) const;
    bool isAssignmentOperator() const;

    std::unique_ptr<BlockStmt> parseBlock();
    std::unique_ptr<Stmt> parseVarDeclStmt(bool isMutable);
    std::unique_ptr<Stmt> parseReturnStmt();
    std::unique_ptr<Stmt> parseDeferStmt();
    std::unique_ptr<Stmt> parseIfStmt();
    std::unique_ptr<Stmt> parseForStmt();
    std::unique_ptr<Stmt> parseAsmStmt();

    std::vector<std::string> parseGenericParamList();
    std::vector<Param> parseParamList();
    std::unique_ptr<FunctionDecl> parseFunctionDecl(bool isExternSignatureOnly);
    std::unique_ptr<Decl> parseStructDecl();
    std::unique_ptr<Decl> parseEnumDecl();
    std::unique_ptr<Decl> parseExternBlockDecl();
    std::unique_ptr<Decl> parseUsingDecl();
    std::vector<std::string> parseAttributes();
};