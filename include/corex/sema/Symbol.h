#pragma once
#include <string>
#include "corex/ast/Type.h"

enum class SymbolKind {
    Variable,
    Function,
    Struct,
    Enum,
    Parameter
};

struct Symbol {
    std::string name;
    SymbolKind kind;
    const Type* type;
    bool isMutable;
    int line;
    int column;

    Symbol(std::string name, SymbolKind kind, const Type* type, bool isMutable, int line, int column)
        : name(std::move(name)), kind(kind), type(type), isMutable(isMutable), line(line), column(column) {}
};