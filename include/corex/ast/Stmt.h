#pragma once
#include <string>
#include <vector>
#include <memory>
#include "corex/ast/Expr.h"
#include "corex/ast/Type.h"

enum class StmtKind {
    VarDecl,
    Return,
    Defer,
    If,
    For,
    Block,
    Asm,
    ExprStmt
};

struct Stmt {
    StmtKind kind;
    int line;
    int column;

    Stmt(StmtKind kind, int line, int column)
        : kind(kind), line(line), column(column) {}
    
    virtual ~Stmt() = default; 
};

struct VarDeclStmt : Stmt {
    std::string name;
    bool isMutable;
    std::unique_ptr<Type> type;
    std::unique_ptr<Expr> initializer;

    VarDeclStmt(std::string name, bool isMutable, std::unique_ptr<Type> type, std::unique_ptr<Expr> initializer, int line, int column)
        : Stmt(StmtKind::VarDecl, line, column), name(std::move(name)), isMutable(isMutable), type(std::move(type)), initializer(std::move(initializer)) {}
};

struct ReturnStmt : Stmt {
    std::vector<std::unique_ptr<Expr>> values;

    ReturnStmt(std::vector<std::unique_ptr<Expr>> values, int line, int column)
        : Stmt(StmtKind::Return, line, column), values(std::move(values)) {}
};

struct DeferStmt : Stmt {
    std::unique_ptr<Expr> expression;

    DeferStmt(std::unique_ptr<Expr> expression, int line, int column)
        : Stmt(StmtKind::Defer, line, column), expression(std::move(expression)) {}
};

struct BlockStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;

    BlockStmt(std::vector<std::unique_ptr<Stmt>> statements, int line, int column)
        : Stmt(StmtKind::Block, line, column), statements(std::move(statements)) {}
};

struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockStmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;

    IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<BlockStmt> thenBranch, std::unique_ptr<Stmt> elseBranch, int line, int column)
        : Stmt(StmtKind::If, line, column), condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}
};

struct ForStmt : Stmt {
    std::unique_ptr<Stmt> initializer;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> increment;
    std::unique_ptr<BlockStmt> body;

    ForStmt(std::unique_ptr<Stmt> initializer, std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> increment, std::unique_ptr<BlockStmt> body, int line, int column)
        : Stmt(StmtKind::For, line, column), initializer(std::move(initializer)), condition(std::move(condition)), increment(std::move(increment)), body(std::move(body)) {}
};

struct AsmStmt : Stmt {
    std::vector<std::string> instructions;

    AsmStmt(std::vector<std::string> instructions, int line, int column)
        : Stmt(StmtKind::Asm, line, column), instructions(std::move(instructions)) {}
};

struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expression;

    ExprStmt(std::unique_ptr<Expr> expression, int line, int column)
        : Stmt(StmtKind::ExprStmt, line, column), expression(std::move(expression)) {}
};
