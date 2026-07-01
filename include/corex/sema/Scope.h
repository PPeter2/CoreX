#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "corex/sema/Symbol.h"

class Scope {
public:
    explicit Scope(Scope* parent);

    void declare(const Symbol& symbol);
    const Symbol* resolve(const std::string& name) const;
    const Symbol* resolveLocal(const std::string& name) const;
    bool isDeclaredLocally(const std::string& name) const;

    Scope* getParent() const;

private:
    Scope* parent;
    std::unordered_map<std::string, Symbol> symbols;
};