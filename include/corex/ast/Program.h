#pragma once
#include <vector>
#include <memory>
#include "corex/ast/Decl.h"

struct Program {
    std::vector<std::unique_ptr<Decl>> declarations;

    explicit Program(std::vector<std::unique_ptr<Decl>> declarations)
        : declarations(std::move(declarations)) {}
};