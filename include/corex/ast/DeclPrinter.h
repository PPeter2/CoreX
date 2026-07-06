#pragma once
#include <string>
#include "corex/ast/Decl.h"
#include "corex/ast/Program.h"

std::string declToString(const Decl* decl, int indent);
std::string programToString(const Program* program);