#include "corex/ast/DeclPrinter.h"
#include "corex/ast/StmtPrinter.h"
#include "corex/ast/TypePrinter.h"

namespace {

std::string indentString(int indent) {
    return std::string(indent * 2, ' ');
}

std::string attributesToString(const std::vector<std::string>& attributes) {
    std::string result;
    for (const auto& attribute : attributes) {
        result += attribute + " ";
    }
    return result;
}

}

std::string declToString(const Decl* decl, int indent) {
    if (decl == nullptr) return indentString(indent) + "null";

    switch (decl->kind) {
        case DeclKind::Function: {
            const FunctionDecl* function = static_cast<const FunctionDecl*>(decl);
            std::string result = indentString(indent) + attributesToString(decl->attributes) + "func " + function->name;

            if (!function->genericParams.empty()) {
                result += "<";
                for (size_t i = 0; i < function->genericParams.size(); i++) {
                    if (i > 0) result += ", ";
                    result += function->genericParams[i];
                }
                result += ">";
            }

            result += "(";
            for (size_t i = 0; i < function->params.size(); i++) {
                if (i > 0) result += ", ";
                result += function->params[i].name + ": " + typeToString(function->params[i].type.get());
            }
            result += ")";

            if (!function->returnTypes.empty()) {
                result += " -> ";
                if (function->returnTypes.size() > 1) result += "(";
                for (size_t i = 0; i < function->returnTypes.size(); i++) {
                    if (i > 0) result += ", ";
                    result += typeToString(function->returnTypes[i].get());
                }
                if (function->returnTypes.size() > 1) result += ")";
            }

            if (function->body != nullptr) {
                result += "\n" + stmtToString(function->body.get(), indent);
            }

            return result;
        }

        case DeclKind::Struct: {
            const StructDecl* structDecl = static_cast<const StructDecl*>(decl);
            std::string result = indentString(indent) + attributesToString(decl->attributes) + "struct " + structDecl->name;

            if (!structDecl->genericParams.empty()) {
                result += "<";
                for (size_t i = 0; i < structDecl->genericParams.size(); i++) {
                    if (i > 0) result += ", ";
                    result += structDecl->genericParams[i];
                }
                result += ">";
            }

            result += " {\n";
            for (const auto& field : structDecl->fields) {
                result += indentString(indent + 1) + field.name + ": " + typeToString(field.type.get()) + "\n";
            }
            result += indentString(indent) + "}";
            return result;
        }

        case DeclKind::Enum: {
            const EnumDecl* enumDecl = static_cast<const EnumDecl*>(decl);
            std::string result = indentString(indent) + attributesToString(decl->attributes) + "enum " + enumDecl->name + " {\n";
            for (const auto& variant : enumDecl->variants) {
                result += indentString(indent + 1) + variant + "\n";
            }
            result += indentString(indent) + "}";
            return result;
        }

        case DeclKind::ExternBlock: {
            const ExternBlockDecl* externBlock = static_cast<const ExternBlockDecl*>(decl);
            std::string result = indentString(indent) + "extern \"" + externBlock->abiName + "\" {\n";
            for (const auto& function : externBlock->functions) {
                result += declToString(function.get(), indent + 1) + "\n";
            }
            result += indentString(indent) + "}";
            return result;
        }

        case DeclKind::Using: {
            const UsingDecl* usingDecl = static_cast<const UsingDecl*>(decl);
            return indentString(indent) + "using \"" + usingDecl->path + "\"";
        }
    }

    return indentString(indent) + "unknown";
}

std::string programToString(const Program* program) {
    std::string result;
    for (size_t i = 0; i < program->declarations.size(); i++) {
        if (i > 0) result += "\n\n";
        result += declToString(program->declarations[i].get(), 0);
    }
    return result;
}