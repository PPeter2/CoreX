#include "corex/sema/Resolver.h"

Resolver::Resolver()
    : globalScope(std::make_unique<Scope>(nullptr)),
      currentScope(globalScope.get()),
      insideFunction(false) {}

std::vector<SemanticError> Resolver::resolve(const Program* program) {
    errors.clear();

    for (const auto& decl : program->declarations) {
        resolveDecl(decl.get());
    }

    return errors;
}

const std::vector<SemanticError>& Resolver::getErrors() const {
    return errors;
}

void Resolver::error(const std::string& message, int line, int column) {
    errors.emplace_back(message, line, column);
}

void Resolver::pushScope() {
    auto child = std::make_unique<Scope>(currentScope);
    currentScope = child.get();
    child.release();
}

void Resolver::popScope() {
    Scope* parent = currentScope->getParent();
    delete currentScope;
    currentScope = parent;
}

void Resolver::resolveDecl(const Decl* decl) {
    switch (decl->kind) {
        case DeclKind::Function:
            resolveFunctionDecl(static_cast<const FunctionDecl*>(decl));
            break;
        case DeclKind::Struct:
            resolveStructDecl(static_cast<const StructDecl*>(decl));
            break;
        case DeclKind::Enum:
            resolveEnumDecl(static_cast<const EnumDecl*>(decl));
            break;
        case DeclKind::ExternBlock:
            resolveExternBlockDecl(static_cast<const ExternBlockDecl*>(decl));
            break;
        case DeclKind::GlobalVar:
            resolveGlobalVarDecl(static_cast<const GlobalVarDecl*>(decl));
            break;
        case DeclKind::Using:
            break;
    }
}

void Resolver::resolveFunctionDecl(const FunctionDecl* decl) {
    if (currentScope->isDeclaredLocally(decl->name)) {
        error("function '" + decl->name + "' is already declared in this scope", decl->line, decl->column);
        return;
    }

    Symbol symbol(decl->name, SymbolKind::Function, nullptr, false, decl->line, decl->column);
    currentScope->declare(symbol);

    if (decl->body != nullptr) {
        pushScope();
        bool savedInsideFunction = insideFunction;
        insideFunction = true;

        for (const auto& param : decl->params) {
            if (currentScope->isDeclaredLocally(param.name)) {
                error("parameter '" + param.name + "' is already declared in this function", decl->line, decl->column);
            } else {
                Symbol paramSymbol(param.name, SymbolKind::Parameter, param.type.get(), true, decl->line, decl->column);
                currentScope->declare(paramSymbol);
            }
        }

        resolveBlock(decl->body.get());
        insideFunction = savedInsideFunction;
        popScope();
    }
}

void Resolver::resolveStructDecl(const StructDecl* decl) {
    if (currentScope->isDeclaredLocally(decl->name)) {
        error("struct '" + decl->name + "' is already declared in this scope", decl->line, decl->column);
        return;
    }

    Symbol symbol(decl->name, SymbolKind::Struct, nullptr, false, decl->line, decl->column);
    currentScope->declare(symbol);
}

void Resolver::resolveEnumDecl(const EnumDecl* decl) {
    if (currentScope->isDeclaredLocally(decl->name)) {
        error("enum '" + decl->name + "' is already declared in this scope", decl->line, decl->column);
        return;
    }

    Symbol symbol(decl->name, SymbolKind::Enum, nullptr, false, decl->line, decl->column);
    currentScope->declare(symbol);
}

void Resolver::resolveExternBlockDecl(const ExternBlockDecl* decl) {
    for (const auto& function : decl->functions) {
        if (currentScope->isDeclaredLocally(function->name)) {
            error("extern function '" + function->name + "' is already declared in this scope", function->line, function->column);
        } else {
            Symbol symbol(function->name, SymbolKind::Function, nullptr, false, function->line, function->column);
            currentScope->declare(symbol);
        }
    }
}

void Resolver::resolveGlobalVarDecl(const GlobalVarDecl* decl) {
    if (currentScope->isDeclaredLocally(decl->name)) {
        error("variable '" + decl->name + "' is already declared in this scope", decl->line, decl->column);
        return;
    }

    if (decl->initializer != nullptr) {
        resolveExpr(decl->initializer.get());
    }

    Symbol symbol(decl->name, SymbolKind::Variable, decl->type.get(), decl->isMutable, decl->line, decl->column);
    currentScope->declare(symbol);
}

void Resolver::resolveStmt(const Stmt* stmt) {
    switch (stmt->kind) {
        case StmtKind::VarDecl:
            resolveVarDeclStmt(static_cast<const VarDeclStmt*>(stmt));
            break;
        case StmtKind::Return:
            resolveReturnStmt(static_cast<const ReturnStmt*>(stmt));
            break;
        case StmtKind::Defer:
            resolveDeferStmt(static_cast<const DeferStmt*>(stmt));
            break;
        case StmtKind::If:
            resolveIfStmt(static_cast<const IfStmt*>(stmt));
            break;
        case StmtKind::For:
            resolveForStmt(static_cast<const ForStmt*>(stmt));
            break;
        case StmtKind::Asm:
            resolveAsmStmt(static_cast<const AsmStmt*>(stmt));
            break;
        case StmtKind::Block:
            pushScope();
            resolveBlock(static_cast<const BlockStmt*>(stmt));
            popScope();
            break;
        case StmtKind::ExprStmt:
            resolveExprStmt(static_cast<const ExprStmt*>(stmt));
            break;
    }
}

void Resolver::resolveBlock(const BlockStmt* stmt) {
    for (const auto& inner : stmt->statements) {
        resolveStmt(inner.get());
    }
}

void Resolver::resolveVarDeclStmt(const VarDeclStmt* stmt) {
    if (stmt->initializer != nullptr) {
        resolveExpr(stmt->initializer.get());
    }

    if (currentScope->isDeclaredLocally(stmt->name)) {
        error("variable '" + stmt->name + "' is already declared in this scope", stmt->line, stmt->column);
        return;
    }

    Symbol symbol(stmt->name, SymbolKind::Variable, stmt->type.get(), stmt->isMutable, stmt->line, stmt->column);
    currentScope->declare(symbol);
}

void Resolver::resolveReturnStmt(const ReturnStmt* stmt) {
    if (!insideFunction) {
        error("'return' used outside of a function", stmt->line, stmt->column);
    }

    for (const auto& value : stmt->values) {
        resolveExpr(value.get());
    }
}

void Resolver::resolveDeferStmt(const DeferStmt* stmt) {
    if (!insideFunction) {
        error("'defer' used outside of a function", stmt->line, stmt->column);
    }

    resolveExpr(stmt->expression.get());
}

void Resolver::resolveIfStmt(const IfStmt* stmt) {
    resolveExpr(stmt->condition.get());

    pushScope();
    resolveBlock(stmt->thenBranch.get());
    popScope();

    if (stmt->elseBranch != nullptr) {
        if (stmt->elseBranch->kind == StmtKind::Block) {
            pushScope();
            resolveBlock(static_cast<const BlockStmt*>(stmt->elseBranch.get()));
            popScope();
        } else {
            resolveStmt(stmt->elseBranch.get());
        }
    }
}

void Resolver::resolveForStmt(const ForStmt* stmt) {
    pushScope();

    if (stmt->initializer != nullptr) {
        resolveStmt(stmt->initializer.get());
    }

    if (stmt->condition != nullptr) {
        resolveExpr(stmt->condition.get());
    }

    if (stmt->increment != nullptr) {
        resolveStmt(stmt->increment.get());
    }

    resolveBlock(stmt->body.get());
    popScope();
}

void Resolver::resolveAsmStmt(const AsmStmt* stmt) {
    if (!insideFunction) {
        error("'asm' used outside of a function", stmt->line, stmt->column);
    }
}

void Resolver::resolveExprStmt(const ExprStmt* stmt) {
    resolveExpr(stmt->expression.get());
}

void Resolver::resolveExpr(const Expr* expr) {
    if (expr == nullptr) return;

    switch (expr->kind) {
        case ExprKind::IntLiteral:
        case ExprKind::FloatLiteral:
        case ExprKind::StringLiteral:
        case ExprKind::CharLiteral:
        case ExprKind::BoolLiteral:
            break;
        case ExprKind::Identifier:
            resolveIdentifierExpr(static_cast<const IdentifierExpr*>(expr));
            break;
        case ExprKind::Binary:
            resolveBinaryExpr(static_cast<const BinaryExpr*>(expr));
            break;
        case ExprKind::Unary:
            resolveUnaryExpr(static_cast<const UnaryExpr*>(expr));
            break;
        case ExprKind::Call:
            resolveCallExpr(static_cast<const CallExpr*>(expr));
            break;
        case ExprKind::Index:
            resolveIndexExpr(static_cast<const IndexExpr*>(expr));
            break;
        case ExprKind::Range:
            resolveRangeExpr(static_cast<const RangeExpr*>(expr));
            break;
        case ExprKind::Assign:
            resolveAssignExpr(static_cast<const AssignExpr*>(expr));
            break;
    }
}

void Resolver::resolveIdentifierExpr(const IdentifierExpr* expr) {
    const Symbol* symbol = currentScope->resolve(expr->name);
    if (symbol == nullptr) {
        error("use of undeclared identifier '" + expr->name + "'", expr->line, expr->column);
    }
}

void Resolver::resolveBinaryExpr(const BinaryExpr* expr) {
    resolveExpr(expr->left.get());
    resolveExpr(expr->right.get());
}

void Resolver::resolveUnaryExpr(const UnaryExpr* expr) {
    resolveExpr(expr->operand.get());
}

void Resolver::resolveCallExpr(const CallExpr* expr) {
    resolveExpr(expr->callee.get());
    for (const auto& argument : expr->arguments) {
        resolveExpr(argument.get());
    }
}

void Resolver::resolveIndexExpr(const IndexExpr* expr) {
    resolveExpr(expr->target.get());
    resolveExpr(expr->index.get());
}

void Resolver::resolveRangeExpr(const RangeExpr* expr) {
    resolveExpr(expr->start.get());
    resolveExpr(expr->end.get());
}

void Resolver::resolveAssignExpr(const AssignExpr* expr) {
    if (expr->target->kind == ExprKind::Identifier) {
        const IdentifierExpr* identifier = static_cast<const IdentifierExpr*>(expr->target.get());
        const Symbol* symbol = currentScope->resolve(identifier->name);

        if (symbol == nullptr) {
            error("use of undeclared identifier '" + identifier->name + "'", identifier->line, identifier->column);
        } else if (!symbol->isMutable) {
            error("cannot assign to immutable variable '" + identifier->name + "'", identifier->line, identifier->column);
        }
    } else {
        resolveExpr(expr->target.get());
    }

    resolveExpr(expr->value.get());
}