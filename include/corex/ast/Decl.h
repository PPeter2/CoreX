#pragma once
#include <string>
#include <vector>
#include <memory>
#include "corex/ast/Type.h"
#include "corex/ast/Stmt.h"
#include "corex/ast/Expr.h"

enum class DeclKind {
    Function,
    Struct,
    Enum,
    ExternBlock,
    Using,
    GlobalVar
};

struct Decl {
    DeclKind kind;
    int line;
    int column;
    std::vector<std::string> attributes;

    Decl(DeclKind kind, int line, int column)
        : kind(kind), line(line), column(column) {}

    virtual ~Decl() = default;
};

struct Param {
    std::string name;
    std::unique_ptr<Type> type;

    Param(std::string name, std::unique_ptr<Type> type)
        : name(std::move(name)), type(std::move(type)) {}
};

struct FunctionDecl : Decl {
    std::string name;
    std::vector<std::string> genericParams;
    std::vector<Param> params;
    std::vector<std::unique_ptr<Type>> returnTypes;
    std::unique_ptr<BlockStmt> body;
    bool isExternSignatureOnly;

    FunctionDecl(std::string name, std::vector<std::string> genericParams, std::vector<Param> params, std::vector<std::unique_ptr<Type>> returnTypes, std::unique_ptr<BlockStmt> body, bool isExternSignatureOnly, int line, int column)
        : Decl(DeclKind::Function, line, column), name(std::move(name)), genericParams(std::move(genericParams)), params(std::move(params)), returnTypes(std::move(returnTypes)), body(std::move(body)), isExternSignatureOnly(isExternSignatureOnly) {}
};

struct Field {
    std::string name;
    std::unique_ptr<Type> type;

    Field(std::string name, std::unique_ptr<Type> type)
        : name(std::move(name)), type(std::move(type)) {}
};

struct StructDecl : Decl {
    std::string name;
    std::vector<std::string> genericParams;
    std::vector<Field> fields;

    StructDecl(std::string name, std::vector<std::string> genericParams, std::vector<Field> fields, int line, int column)
        : Decl(DeclKind::Struct, line, column), name(std::move(name)), genericParams(std::move(genericParams)), fields(std::move(fields)) {}
};

struct EnumDecl : Decl {
    std::string name;
    std::vector<std::string> variants;

    EnumDecl(std::string name, std::vector<std::string> variants, int line, int column)
        : Decl(DeclKind::Enum, line, column), name(std::move(name)), variants(std::move(variants)) {}
};

struct ExternBlockDecl : Decl {
    std::string abiName;
    std::vector<std::unique_ptr<FunctionDecl>> functions;

    ExternBlockDecl(std::string abiName, std::vector<std::unique_ptr<FunctionDecl>> functions, int line, int column)
        : Decl(DeclKind::ExternBlock, line, column), abiName(std::move(abiName)), functions(std::move(functions)) {}
};

struct UsingDecl : Decl {
    std::string path;

    UsingDecl(std::string path, int line, int column)
        : Decl(DeclKind::Using, line, column), path(std::move(path)) {}
};

struct GlobalVarDecl : Decl {
    std::string name;
    bool isMutable;
    std::unique_ptr<Type> type;
    std::unique_ptr<Expr> initializer;

    GlobalVarDecl(std::string name, bool isMutable, std::unique_ptr<Type> type, std::unique_ptr<Expr> initializer, int line, int column)
        : Decl(DeclKind::GlobalVar, line, column), name(std::move(name)), isMutable(isMutable), type(std::move(type)), initializer(std::move(initializer)) {}
};