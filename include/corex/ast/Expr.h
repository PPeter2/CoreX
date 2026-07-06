#pragma once
#include <string>
#include <vector>
#include <memory>

enum class ExprKind {
    IntLiteral,
    FloatLiteral,
    StringLiteral,
    CharLiteral,
    BoolLiteral,
    Identifier,
    Binary,
    Unary,
    Call,
    Index,
    Range,
    Assign
};

struct Expr {
    ExprKind kind;
    int line;
    int column;

    Expr(ExprKind kind, int line, int column)
        : kind(kind), line(line), column(column) {}

    virtual ~Expr() = default;
};

struct IntLiteralExpr : Expr {
    std::string value;

    IntLiteralExpr(std::string value, int line, int column)
        : Expr(ExprKind::IntLiteral, line, column), value(std::move(value)) {}
};

struct FloatLiteralExpr : Expr {
    std::string value;

    FloatLiteralExpr(std::string value, int line, int column)
        : Expr(ExprKind::FloatLiteral, line, column), value(std::move(value)) {}
};

struct StringLiteralExpr : Expr {
    std::string value;

    StringLiteralExpr(std::string value, int line, int column)
        : Expr(ExprKind::StringLiteral, line, column), value(std::move(value)) {}
};

struct CharLiteralExpr : Expr {
    char value;

    CharLiteralExpr(char value, int line, int column)
        : Expr(ExprKind::CharLiteral, line, column), value(value) {}
};

struct BoolLiteralExpr : Expr {
    bool value;

    BoolLiteralExpr(bool value, int line, int column)
        : Expr(ExprKind::BoolLiteral, line, column), value(value) {}
};

struct IdentifierExpr : Expr {
    std::string name;

    IdentifierExpr(std::string name, int line, int column)
        : Expr(ExprKind::Identifier, line, column), name(std::move(name)) {}
};

struct BinaryExpr : Expr {
    std::string op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::string op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, int line, int column)
        : Expr(ExprKind::Binary, line, column), op(std::move(op)), left(std::move(left)), right(std::move(right)) {}
};

struct UnaryExpr : Expr {
    std::string op;
    std::unique_ptr<Expr> operand;

    UnaryExpr(std::string op, std::unique_ptr<Expr> operand, int line, int column)
        : Expr(ExprKind::Unary, line, column), op(std::move(op)), operand(std::move(operand)) {}
};

struct CallExpr : Expr {
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> arguments;

    CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> arguments, int line, int column)
        : Expr(ExprKind::Call, line, column), callee(std::move(callee)), arguments(std::move(arguments)) {}
};

struct IndexExpr : Expr {
    std::unique_ptr<Expr> target;
    std::unique_ptr<Expr> index;

    IndexExpr(std::unique_ptr<Expr> target, std::unique_ptr<Expr> index, int line, int column)
        : Expr(ExprKind::Index, line, column), target(std::move(target)), index(std::move(index)) {}
};

struct RangeExpr : Expr {
    std::unique_ptr<Expr> start;
    std::unique_ptr<Expr> end;
    bool inclusive;

    RangeExpr(std::unique_ptr<Expr> start, std::unique_ptr<Expr> end, bool inclusive, int line, int column)
        : Expr(ExprKind::Range, line, column), start(std::move(start)), end(std::move(end)), inclusive(inclusive) {}
};

struct AssignExpr : Expr {
    std::string op;
    std::unique_ptr<Expr> target;
    std::unique_ptr<Expr> value;

    AssignExpr(std::string op, std::unique_ptr<Expr> target, std::unique_ptr<Expr> value, int line, int column)
        : Expr(ExprKind::Assign, line, column), op(std::move(op)), target(std::move(target)), value(std::move(value)) {}
};