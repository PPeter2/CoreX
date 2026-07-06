#include "corex/ast/StmtPrinter.h"
#include "corex/ast/AstPrinter.h"
#include "corex/ast/TypePrinter.h"

namespace {

std::string indentString(int indent) {
    return std::string(indent * 2, ' ');
}

}

std::string stmtToString(const Stmt* stmt, int indent) {
    if (stmt == nullptr) return indentString(indent) + "null";

    switch (stmt->kind) {
        case StmtKind::VarDecl: {
            const VarDeclStmt* varDecl = static_cast<const VarDeclStmt*>(stmt);
            std::string result = indentString(indent) + (varDecl->isMutable ? "mut " : "let ") + varDecl->name;
            if (varDecl->type != nullptr) {
                result += ": " + typeToString(varDecl->type.get());
            }
            if (varDecl->initializer != nullptr) {
                result += " = " + exprToString(varDecl->initializer.get());
            }
            return result;
        }

        case StmtKind::Return: {
            const ReturnStmt* returnStmt = static_cast<const ReturnStmt*>(stmt);
            std::string result = indentString(indent) + "return";
            for (const auto& value : returnStmt->values) {
                result += " " + exprToString(value.get());
            }
            return result;
        }

        case StmtKind::Defer: {
            const DeferStmt* deferStmt = static_cast<const DeferStmt*>(stmt);
            return indentString(indent) + "defer " + exprToString(deferStmt->expression.get());
        }

        case StmtKind::Block: {
            const BlockStmt* block = static_cast<const BlockStmt*>(stmt);
            std::string result = indentString(indent) + "{\n";
            for (const auto& inner : block->statements) {
                result += stmtToString(inner.get(), indent + 1) + "\n";
            }
            result += indentString(indent) + "}";
            return result;
        }

        case StmtKind::If: {
            const IfStmt* ifStmt = static_cast<const IfStmt*>(stmt);
            std::string result = indentString(indent) + "if " + exprToString(ifStmt->condition.get()) + "\n";
            result += stmtToString(ifStmt->thenBranch.get(), indent);
            if (ifStmt->elseBranch != nullptr) {
                result += "\n" + indentString(indent) + "else\n";
                result += stmtToString(ifStmt->elseBranch.get(), indent);
            }
            return result;
        }

        case StmtKind::For: {
            const ForStmt* forStmt = static_cast<const ForStmt*>(stmt);
            std::string result = indentString(indent) + "for ";
            result += forStmt->initializer != nullptr ? stmtToString(forStmt->initializer.get(), 0) : "";
            result += "; ";
            result += forStmt->condition != nullptr ? exprToString(forStmt->condition.get()) : "";
            result += "; ";
            result += forStmt->increment != nullptr ? stmtToString(forStmt->increment.get(), 0) : "";
            result += "\n" + stmtToString(forStmt->body.get(), indent);
            return result;
        }

        case StmtKind::Asm: {
            const AsmStmt* asmStmt = static_cast<const AsmStmt*>(stmt);
            std::string result = indentString(indent) + "asm {\n";
            for (const auto& instruction : asmStmt->instructions) {
                result += indentString(indent + 1) + "\"" + instruction + "\"\n";
            }
            result += indentString(indent) + "}";
            return result;
        }

        case StmtKind::ExprStmt: {
            const ExprStmt* exprStmt = static_cast<const ExprStmt*>(stmt);
            return indentString(indent) + exprToString(exprStmt->expression.get());
        }
    }

    return indentString(indent) + "unknown";
}