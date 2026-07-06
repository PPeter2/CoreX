#include "corex/ast/AstPrinter.h"

std::string exprToString(const Expr* expr) {
    if (expr == nullptr) return "null";

    switch (expr->kind) {
        case ExprKind::IntLiteral:
            return static_cast<const IntLiteralExpr*>(expr)->value;

        case ExprKind::FloatLiteral:
            return static_cast<const FloatLiteralExpr*>(expr)->value;

        case ExprKind::StringLiteral:
            return "\"" + static_cast<const StringLiteralExpr*>(expr)->value + "\"";

        case ExprKind::CharLiteral:
            return std::string("'") + static_cast<const CharLiteralExpr*>(expr)->value + "'";

        case ExprKind::BoolLiteral:
            return static_cast<const BoolLiteralExpr*>(expr)->value ? "true" : "false";

        case ExprKind::Identifier:
            return static_cast<const IdentifierExpr*>(expr)->name;

        case ExprKind::Binary: {
            const BinaryExpr* binary = static_cast<const BinaryExpr*>(expr);
            return "(" + binary->op + " " + exprToString(binary->left.get()) + " " + exprToString(binary->right.get()) + ")";
        }

        case ExprKind::Unary: {
            const UnaryExpr* unary = static_cast<const UnaryExpr*>(expr);
            return "(" + unary->op + " " + exprToString(unary->operand.get()) + ")";
        }

        case ExprKind::Call: {
            const CallExpr* call = static_cast<const CallExpr*>(expr);
            std::string result = "(call " + exprToString(call->callee.get());
            for (const auto& argument : call->arguments) {
                result += " " + exprToString(argument.get());
            }
            result += ")";
            return result;
        }

        case ExprKind::Index: {
            const IndexExpr* index = static_cast<const IndexExpr*>(expr);
            return "(index " + exprToString(index->target.get()) + " " + exprToString(index->index.get()) + ")";
        }

        case ExprKind::Range: {
            const RangeExpr* range = static_cast<const RangeExpr*>(expr);
            std::string op = range->inclusive ? "..=" : "..";
            return "(" + op + " " + exprToString(range->start.get()) + " " + exprToString(range->end.get()) + ")";
        }

        case ExprKind::Assign: {
            const AssignExpr* assign = static_cast<const AssignExpr*>(expr);
            return "(" + assign->op + " " + exprToString(assign->target.get()) + " " + exprToString(assign->value.get()) + ")";
        }
    }

    return "unknown";
}