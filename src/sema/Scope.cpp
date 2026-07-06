#include "corex/sema/Scope.h"

Scope::Scope(Scope* parent)
    : parent(parent) {}

void Scope::declare(const Symbol& symbol) {
    symbols.emplace(symbol.name, symbol);
}

const Symbol* Scope::resolve(const std::string& name) const {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return &it->second;
    }

    if (parent != nullptr) {
        return parent->resolve(name);
    }

    return nullptr;
}

const Symbol* Scope::resolveLocal(const std::string& name) const {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return &it->second;
    }
    return nullptr;
}

bool Scope::isDeclaredLocally(const std::string& name) const {
    return symbols.find(name) != symbols.end();
}

Scope* Scope::getParent() const {
    return parent;
}