#pragma once
#include <vector>
#include <string>
#include <memory>
#include "corex/ast/Program.h"
#include "corex/ast/Decl.h"
#include "corex/ast/Stmt.h"
#include "corex/ast/Expr.h"
#include "corex/sema/Scope.h"

struct SemanticError {
    std::string message;
    int line;
    int column;

    SemanticError(std::string message, int line, int column)
        : message(std::move(message)), line(line), column(column) {}
};

class Resolver {
public:
    Resolver();

    std::vector<SemanticError> resolve(const Program* program);
    const std::vector<SemanticError>& getErrors() const;

private:
    std::unique_ptr<Scope> globalScope;
    Scope* currentScope;
    std::vector<SemanticError> errors;
    bool insideFunction;

    void error(const std::string& message, int line, int column);

    void pushScope();
    void popScope();

    void resolveDecl(const Decl* decl);
    void resolveFunctionDecl(const FunctionDecl* decl);
    void resolveStructDecl(const StructDecl* decl);
    void resolveEnumDecl(const EnumDecl* decl);
    void resolveExternBlockDecl(const ExternBlockDecl* decl);
    void resolveGlobalVarDecl(const GlobalVarDecl* decl);

    void resolveStmt(const Stmt* stmt);
    void resolveBlock(const BlockStmt* stmt);
    void resolveVarDeclStmt(const VarDeclStmt* stmt);
    void resolveReturnStmt(const ReturnStmt* stmt);
    void resolveDeferStmt(const DeferStmt* stmt);
    void resolveIfStmt(const IfStmt* stmt);
    void resolveForStmt(const ForStmt* stmt);
    void resolveAsmStmt(const AsmStmt* stmt);
    void resolveExprStmt(const ExprStmt* stmt);

    void resolveExpr(const Expr* expr);
    void resolveIdentifierExpr(const IdentifierExpr* expr);
    void resolveBinaryExpr(const BinaryExpr* expr);
    void resolveUnaryExpr(const UnaryExpr* expr);
    void resolveCallExpr(const CallExpr* expr);
    void resolveIndexExpr(const IndexExpr* expr);
    void resolveRangeExpr(const RangeExpr* expr);
    void resolveAssignExpr(const AssignExpr* expr);
};