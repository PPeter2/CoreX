#include "corex/codegen/CodeGenX64.h"
#include <stdexcept>
#include <sstream>

CodeGenX64::CodeGenX64()
    : labelCounter(0), stringCounter(0), nextStackOffset(0) {}

void CodeGenX64::error(const std::string& message, int line, int column) const {
    std::ostringstream oss;
    oss << "codegen error at " << line << ":" << column << " - " << message;
    throw std::runtime_error(oss.str());
}

std::string CodeGenX64::newLabel(const std::string& hint) {
    return ".L" + hint + "_" + std::to_string(labelCounter++);
}

void CodeGenX64::emit(const std::string& line) {
    functionBuffer += "    " + line + "\n";
}

void CodeGenX64::emitLabel(const std::string& label) {
    functionBuffer += label + ":\n";
}

void CodeGenX64::pushScope() {
    scopes.emplace_back();
}

void CodeGenX64::popScope() {
    scopes.pop_back();
}

int CodeGenX64::declareLocal(const std::string& name, int line, int column) {
    if (scopes.empty()) {
        error("internal error: no active scope for local '" + name + "'", line, column);
    }
    nextStackOffset += 8;
    int offset = -nextStackOffset;
    scopes.back()[name] = offset;
    return offset;
}

int CodeGenX64::lookupLocal(const std::string& name, int line, int column) const {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second;
        }
    }
    error("codegen internal error: '" + name + "' was not found as a local variable "
          "(it may refer to a function value, a global, or a construct codegen does not support yet)", line, column);
}

std::string CodeGenX64::addStringConstant(const std::string& value) {
    std::string label = ".LC" + std::to_string(stringCounter++);
    dataSection += label + ":\n";
    dataSection += "    .string \"";
    for (unsigned char c : value) {
        switch (c) {
            case '\n': dataSection += "\\n"; break;
            case '\t': dataSection += "\\t"; break;
            case '\r': dataSection += "\\r"; break;
            case '\\': dataSection += "\\\\"; break;
            case '"':  dataSection += "\\\""; break;
            default:
                if (c < 0x20 || c == 0x7f) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\%03o", c);
                    dataSection += buf;
                } else {
                    dataSection += static_cast<char>(c);
                }
        }
    }
    dataSection += "\"\n";
    return label;
}

bool CodeGenX64::isSupportedType(const Type* type) const {
    if (type == nullptr) return true;
    if (type->kind == TypeKind::Pointer) return true;
    if (type->kind != TypeKind::Named) return false;
    const std::string& name = static_cast<const NamedType*>(type)->name;
    return name == "int" || name == "i8" || name == "i16" || name == "i32" || name == "i64" ||
           name == "uint" || name == "u8" || name == "u16" || name == "u32" || name == "u64" ||
           name == "char" || name == "boolean" || name == "string" || name == "void";
}

std::string CodeGenX64::describeUnsupportedType(const Type* type) const {
    if (type == nullptr) return "<none>";
    switch (type->kind) {
        case TypeKind::Named:    return static_cast<const NamedType*>(type)->name;
        case TypeKind::Pointer:  return "pointer type";
        case TypeKind::Array:    return "array type";
        case TypeKind::Slice:    return "slice type";
        case TypeKind::Generic:  return "generic type";
        case TypeKind::Function: return "function-pointer type";
    }
    return "unknown type";
}

void CodeGenX64::checkArity(const std::string& name, size_t argCount, int line, int column) const {
    auto it = functionArity.find(name);
    if (it == functionArity.end()) {
        return;
    }
    if (it->second != argCount) {
        std::ostringstream oss;
        oss << "'" << name << "' expects " << it->second << " argument(s) but got " << argCount;
        error(oss.str(), line, column);
    }
}

void CodeGenX64::collectSignatures(const Program* program) {
    for (const auto& decl : program->declarations) {
        if (decl->kind == DeclKind::Function) {
            const FunctionDecl* fn = static_cast<const FunctionDecl*>(decl.get());
            functionArity[fn->name] = fn->params.size();
        } else if (decl->kind == DeclKind::ExternBlock) {
            const ExternBlockDecl* block = static_cast<const ExternBlockDecl*>(decl.get());
            for (const auto& fn : block->functions) {
                functionArity[fn->name] = fn->params.size();
            }
        }
    }
}

std::string CodeGenX64::generate(const Program* program) {
    textSection.clear();
    dataSection.clear();
    functionArity.clear();

    collectSignatures(program);

    for (const auto& decl : program->declarations) {
        if (decl->kind == DeclKind::Function) {
            genFunction(static_cast<const FunctionDecl*>(decl.get()));
        }
    }

    std::ostringstream out;
    out << "    .intel_syntax noprefix\n";
    out << "    .text\n";
    out << textSection;
    if (!dataSection.empty()) {
        out << "    .section .rodata\n";
        out << dataSection;
    }
    out << "    .section .note.GNU-stack,\"\",@progbits\n";
    return out.str();
}

namespace {

std::string slotOperand(int offset) {
    if (offset < 0) return "[rbp - " + std::to_string(-offset) + "]";
    return "[rbp + " + std::to_string(offset) + "]";
}

const char* kArgRegisters[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

long long parseIntLiteral(const std::string& raw) {
    std::string s;
    for (char c : raw) {
        if (c != '_') s += c;
    }
    if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        return std::stoll(s.substr(2), nullptr, 16);
    }
    if (s.size() > 2 && s[0] == '0' && (s[1] == 'o' || s[1] == 'O')) {
        return std::stoll(s.substr(2), nullptr, 8);
    }
    if (s.size() > 2 && s[0] == '0' && (s[1] == 'b' || s[1] == 'B')) {
        return std::stoll(s.substr(2), nullptr, 2);
    }
    return std::stoll(s, nullptr, 10);
}

}

void CodeGenX64::genFunction(const FunctionDecl* decl) {
    if (decl->body == nullptr) {
        return;
    }

    if (decl->params.size() > 6) {
        error("functions with more than 6 parameters are not yet supported by codegen", decl->line, decl->column);
    }
    for (const auto& param : decl->params) {
        if (!isSupportedType(param.type.get())) {
            error("parameter '" + param.name + "' has a type codegen does not support yet (" +
                  describeUnsupportedType(param.type.get()) + ")", decl->line, decl->column);
        }
    }
    if (decl->returnTypes.size() > 1) {
        error("function '" + decl->name + "' returns multiple values, which codegen does not support yet "
              "(single-value returns only, for now)", decl->line, decl->column);
    }
    if (!decl->returnTypes.empty() && !isSupportedType(decl->returnTypes[0].get())) {
        error("function '" + decl->name + "' has a return type codegen does not support yet (" +
              describeUnsupportedType(decl->returnTypes[0].get()) + ")", decl->line, decl->column);
    }

    functionBuffer.clear();
    scopes.clear();
    nextStackOffset = 0;
    epilogueLabel = newLabel(decl->name + "_epilogue");

    emit(".globl " + decl->name);
    emitLabel(decl->name);
    emit("push rbp");
    emit("mov rbp, rsp");
    emit("sub rsp, __FRAME_SIZE__");
    emit("mov rax, 0");

    pushScope();
    for (size_t i = 0; i < decl->params.size(); i++) {
        genParamBinding(decl->params[i], static_cast<int>(i));
    }

    genStatements(decl->body->statements);
    popScope();

    emitLabel(epilogueLabel);
    emit("leave");
    emit("ret");

    int frameSize = nextStackOffset;
    if (frameSize % 16 != 0) {
        frameSize += 16 - (frameSize % 16);
    }

    std::string finalized = functionBuffer;
    std::string placeholder = "__FRAME_SIZE__";
    size_t pos = finalized.find(placeholder);
    if (pos != std::string::npos) {
        finalized.replace(pos, placeholder.size(), std::to_string(frameSize));
    }

    textSection += finalized;
    textSection += "\n";
}

void CodeGenX64::genParamBinding(const Param& param, int index) {
    int offset = declareLocal(param.name, param.type ? param.type->line : 0, param.type ? param.type->column : 0);
    emit("mov " + slotOperand(offset) + ", " + kArgRegisters[index]);
}

void CodeGenX64::genStatements(const std::vector<std::unique_ptr<Stmt>>& stmts) {
    for (const auto& stmt : stmts) {
        genStmt(stmt.get());
    }
}

void CodeGenX64::genStmt(const Stmt* stmt) {
    switch (stmt->kind) {
        case StmtKind::VarDecl:
            genVarDeclStmt(static_cast<const VarDeclStmt*>(stmt));
            return;
        case StmtKind::Return:
            genReturnStmt(static_cast<const ReturnStmt*>(stmt));
            return;
        case StmtKind::Defer:
            error("'defer' is not yet supported by codegen (it needs cleanup-path generation at every "
                  "return point, which is planned for a follow-up phase)", stmt->line, stmt->column);
        case StmtKind::Block: {
            pushScope();
            genStatements(static_cast<const BlockStmt*>(stmt)->statements);
            popScope();
            return;
        }
        case StmtKind::If:
            genIfStmt(static_cast<const IfStmt*>(stmt));
            return;
        case StmtKind::For:
            genForStmt(static_cast<const ForStmt*>(stmt));
            return;
        case StmtKind::Asm:
            genAsmStmt(static_cast<const AsmStmt*>(stmt));
            return;
        case StmtKind::ExprStmt:
            genExprStmt(static_cast<const ExprStmt*>(stmt));
            return;
    }
}

void CodeGenX64::genVarDeclStmt(const VarDeclStmt* stmt) {
    if (stmt->type != nullptr && !isSupportedType(stmt->type.get())) {
        error("variable '" + stmt->name + "' has a type codegen does not support yet (" +
              describeUnsupportedType(stmt->type.get()) + ")", stmt->line, stmt->column);
    }

    int offset = declareLocal(stmt->name, stmt->line, stmt->column);

    if (stmt->initializer != nullptr) {
        genExpr(stmt->initializer.get());
        emit("mov " + slotOperand(offset) + ", rax");
    }
}

void CodeGenX64::genReturnStmt(const ReturnStmt* stmt) {
    if (stmt->values.size() > 1) {
        error("returning multiple values is not yet supported by codegen", stmt->line, stmt->column);
    }
    if (stmt->values.size() == 1) {
        genExpr(stmt->values[0].get());
    } else {
        emit("mov rax, 0");
    }
    emit("jmp " + epilogueLabel);
}

void CodeGenX64::genIfStmt(const IfStmt* stmt) {
    std::string elseLabel = newLabel("else");
    std::string endLabel = newLabel("endif");

    genJumpIfFalse(stmt->condition.get(), stmt->elseBranch != nullptr ? elseLabel : endLabel);

    pushScope();
    genStatements(stmt->thenBranch->statements);
    popScope();

    if (stmt->elseBranch != nullptr) {
        emit("jmp " + endLabel);
        emitLabel(elseLabel);
        if (stmt->elseBranch->kind == StmtKind::Block) {
            pushScope();
            genStatements(static_cast<const BlockStmt*>(stmt->elseBranch.get())->statements);
            popScope();
        } else {
            genStmt(stmt->elseBranch.get());
        }
    }

    emitLabel(endLabel);
}

void CodeGenX64::genForStmt(const ForStmt* stmt) {
    std::string loopStart = newLabel("for_start");
    std::string loopEnd = newLabel("for_end");

    pushScope();

    if (stmt->initializer != nullptr) {
        genStmt(stmt->initializer.get());
    }

    emitLabel(loopStart);

    if (stmt->condition != nullptr) {
        genJumpIfFalse(stmt->condition.get(), loopEnd);
    }

    genStatements(stmt->body->statements);

    if (stmt->increment != nullptr) {
        genStmt(stmt->increment.get());
    }

    emit("jmp " + loopStart);
    emitLabel(loopEnd);

    popScope();
}

void CodeGenX64::genAsmStmt(const AsmStmt* stmt) {
    for (const auto& instruction : stmt->instructions) {
        emit(instruction);
    }
}

void CodeGenX64::genExprStmt(const ExprStmt* stmt) {
    genExpr(stmt->expression.get());
}

void CodeGenX64::genExpr(const Expr* expr) {
    switch (expr->kind) {
        case ExprKind::IntLiteral: {
            long long value = parseIntLiteral(static_cast<const IntLiteralExpr*>(expr)->value);
            emit("mov rax, " + std::to_string(value));
            return;
        }
        case ExprKind::FloatLiteral:
            error("floating point values are not yet supported by codegen", expr->line, expr->column);
        case ExprKind::StringLiteral: {
            std::string label = addStringConstant(static_cast<const StringLiteralExpr*>(expr)->value);
            emit("lea rax, [rip + " + label + "]");
            return;
        }
        case ExprKind::CharLiteral: {
            char c = static_cast<const CharLiteralExpr*>(expr)->value;
            emit("mov rax, " + std::to_string(static_cast<int>(static_cast<unsigned char>(c))));
            return;
        }
        case ExprKind::BoolLiteral: {
            bool v = static_cast<const BoolLiteralExpr*>(expr)->value;
            emit(std::string("mov rax, ") + (v ? "1" : "0"));
            return;
        }
        case ExprKind::Identifier: {
            const std::string& name = static_cast<const IdentifierExpr*>(expr)->name;
            int offset = lookupLocal(name, expr->line, expr->column);
            emit("mov rax, " + slotOperand(offset));
            return;
        }
        case ExprKind::Binary:
            genBinaryExpr(static_cast<const BinaryExpr*>(expr));
            return;
        case ExprKind::Unary:
            genUnaryExpr(static_cast<const UnaryExpr*>(expr));
            return;
        case ExprKind::Call:
            genCallExpr(static_cast<const CallExpr*>(expr));
            return;
        case ExprKind::Index:
            error("array/slice indexing is not yet supported by codegen", expr->line, expr->column);
        case ExprKind::Range:
            error("range expressions are only meaningful inside indexing right now, "
                  "which codegen does not support yet", expr->line, expr->column);
        case ExprKind::Assign:
            genAssignExpr(static_cast<const AssignExpr*>(expr));
            return;
    }
}

void CodeGenX64::genBinaryExpr(const BinaryExpr* expr) {
    const std::string& op = expr->op;

    if (op == "&&" || op == "||" || op == "==" || op == "!=" ||
        op == "<" || op == ">" || op == "<=" || op == ">=") {
        materializeBool(expr);
        return;
    }

    genExpr(expr->left.get());
    emit("push rax");
    genExpr(expr->right.get());
    emit("mov rcx, rax");
    emit("pop rax");

    if (op == "+") { emit("add rax, rcx"); return; }
    if (op == "-") { emit("sub rax, rcx"); return; }
    if (op == "*") { emit("imul rax, rcx"); return; }
    if (op == "/") { emit("cqo"); emit("idiv rcx"); return; }
    if (op == "%") { emit("cqo"); emit("idiv rcx"); emit("mov rax, rdx"); return; }
    if (op == "|") { emit("or rax, rcx"); return; }
    if (op == "^") { emit("xor rax, rcx"); return; }
    if (op == "&") { emit("and rax, rcx"); return; }
    if (op == "<<") { emit("sal rax, cl"); return; }
    if (op == ">>") { emit("sar rax, cl"); return; }

    error("binary operator '" + op + "' is not yet supported by codegen", expr->line, expr->column);
}

void CodeGenX64::genUnaryExpr(const UnaryExpr* expr) {
    const std::string& op = expr->op;

    if (op == "!") {
        materializeBool(expr);
        return;
    }
    if (op == "-") {
        genExpr(expr->operand.get());
        emit("neg rax");
        return;
    }
    if (op == "~") {
        genExpr(expr->operand.get());
        emit("not rax");
        return;
    }
    if (op == "*") {
        error("pointer dereference ('*') is not yet supported by codegen", expr->line, expr->column);
    }
    if (op == "&") {
        error("address-of ('&') is not yet supported by codegen", expr->line, expr->column);
    }

    error("unary operator '" + op + "' is not yet supported by codegen", expr->line, expr->column);
}

void CodeGenX64::genCallExpr(const CallExpr* expr) {
    if (expr->callee->kind != ExprKind::Identifier) {
        error("codegen only supports calling a plain named function right now", expr->line, expr->column);
    }
    const std::string& name = static_cast<const IdentifierExpr*>(expr->callee.get())->name;

    if (expr->arguments.size() > 6) {
        error("calls with more than 6 arguments are not yet supported by codegen", expr->line, expr->column);
    }
    checkArity(name, expr->arguments.size(), expr->line, expr->column);

    for (const auto& arg : expr->arguments) {
        genExpr(arg.get());
        emit("push rax");
    }

    for (size_t i = expr->arguments.size(); i-- > 0; ) {
        emit(std::string("pop ") + kArgRegisters[i]);
    }

    emit("call " + name);
}

void CodeGenX64::genAssignExpr(const AssignExpr* expr) {
    if (expr->target->kind != ExprKind::Identifier) {
        error("codegen only supports assigning to a plain variable right now "
              "(not array elements, pointers, or struct fields)", expr->line, expr->column);
    }
    const std::string& name = static_cast<const IdentifierExpr*>(expr->target.get())->name;
    int offset = lookupLocal(name, expr->line, expr->column);

    if (expr->op == "=") {
        genExpr(expr->value.get());
        emit("mov " + slotOperand(offset) + ", rax");
        return;
    }

    std::string baseOp = expr->op.substr(0, expr->op.size() - 1);

    emit("mov rax, " + slotOperand(offset));
    emit("push rax");
    genExpr(expr->value.get());
    emit("mov rcx, rax");
    emit("pop rax");

    if (baseOp == "+") { emit("add rax, rcx"); }
    else if (baseOp == "-") { emit("sub rax, rcx"); }
    else if (baseOp == "*") { emit("imul rax, rcx"); }
    else if (baseOp == "/") { emit("cqo"); emit("idiv rcx"); }
    else if (baseOp == "%") { emit("cqo"); emit("idiv rcx"); emit("mov rax, rdx"); }
    else {
        error("compound assignment operator '" + expr->op + "' is not yet supported by codegen", expr->line, expr->column);
    }

    emit("mov " + slotOperand(offset) + ", rax");
}

void CodeGenX64::materializeBool(const Expr* expr) {
    std::string trueLabel = newLabel("true");
    std::string endLabel = newLabel("boolend");

    genJumpIfTrue(expr, trueLabel);
    emit("mov rax, 0");
    emit("jmp " + endLabel);
    emitLabel(trueLabel);
    emit("mov rax, 1");
    emitLabel(endLabel);
}

void CodeGenX64::genComparisonJump(const BinaryExpr* expr, const std::string& label, bool jumpWhenTrue) {
    genExpr(expr->left.get());
    emit("push rax");
    genExpr(expr->right.get());
    emit("mov rcx, rax");
    emit("pop rax");
    emit("cmp rax, rcx");

    const std::string& op = expr->op;
    std::string trueJump, falseJump;

    if (op == "==") { trueJump = "je"; falseJump = "jne"; }
    else if (op == "!=") { trueJump = "jne"; falseJump = "je"; }
    else if (op == "<") { trueJump = "jl"; falseJump = "jge"; }
    else if (op == ">") { trueJump = "jg"; falseJump = "jle"; }
    else if (op == "<=") { trueJump = "jle"; falseJump = "jg"; }
    else if (op == ">=") { trueJump = "jge"; falseJump = "jl"; }
    else { error("unsupported comparison operator '" + op + "'", expr->line, expr->column); }

    emit((jumpWhenTrue ? trueJump : falseJump) + " " + label);
}

void CodeGenX64::genJumpIfFalse(const Expr* expr, const std::string& label) {
    if (expr->kind == ExprKind::Binary) {
        const BinaryExpr* bin = static_cast<const BinaryExpr*>(expr);
        if (bin->op == "&&") {
            genJumpIfFalse(bin->left.get(), label);
            genJumpIfFalse(bin->right.get(), label);
            return;
        }
        if (bin->op == "||") {
            std::string trueLabel = newLabel("or_true");
            genJumpIfTrue(bin->left.get(), trueLabel);
            genJumpIfFalse(bin->right.get(), label);
            emitLabel(trueLabel);
            return;
        }
        if (bin->op == "==" || bin->op == "!=" || bin->op == "<" || bin->op == ">" || bin->op == "<=" || bin->op == ">=") {
            genComparisonJump(bin, label, false);
            return;
        }
    }
    if (expr->kind == ExprKind::Unary) {
        const UnaryExpr* un = static_cast<const UnaryExpr*>(expr);
        if (un->op == "!") {
            genJumpIfTrue(un->operand.get(), label);
            return;
        }
    }

    genExpr(expr);
    emit("cmp rax, 0");
    emit("je " + label);
}

void CodeGenX64::genJumpIfTrue(const Expr* expr, const std::string& label) {
    if (expr->kind == ExprKind::Binary) {
        const BinaryExpr* bin = static_cast<const BinaryExpr*>(expr);
        if (bin->op == "&&") {
            std::string falseLabel = newLabel("and_false");
            genJumpIfFalse(bin->left.get(), falseLabel);
            genJumpIfTrue(bin->right.get(), label);
            emitLabel(falseLabel);
            return;
        }
        if (bin->op == "||") {
            genJumpIfTrue(bin->left.get(), label);
            genJumpIfTrue(bin->right.get(), label);
            return;
        }
        if (bin->op == "==" || bin->op == "!=" || bin->op == "<" || bin->op == ">" || bin->op == "<=" || bin->op == ">=") {
            genComparisonJump(bin, label, true);
            return;
        }
    }
    if (expr->kind == ExprKind::Unary) {
        const UnaryExpr* un = static_cast<const UnaryExpr*>(expr);
        if (un->op == "!") {
            genJumpIfFalse(un->operand.get(), label);
            return;
        }
    }

    genExpr(expr);
    emit("cmp rax, 0");
    emit("jne " + label);
}