#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "corex/ast/Program.h"
#include "corex/ast/Decl.h"
#include "corex/ast/Stmt.h"
#include "corex/ast/Expr.h"
#include "corex/ast/Type.h"
#include "corex/codegen/CodegenTarget.h"

class CodeGenX64 {
public:
    explicit CodeGenX64(CodegenTarget target = CodegenTarget::hostDefault());

    std::string generate(const Program* program);

private:
    CodegenTarget target;

    std::unordered_map<std::string, size_t> functionArity;

    std::string textSection;
    std::string dataSection;
    std::string functionBuffer;
    int labelCounter;
    int stringCounter;

    std::vector<std::unordered_map<std::string, int>> scopes;
    int nextStackOffset;
    std::string epilogueLabel;

    std::string newLabel(const std::string& hint);
    void emit(const std::string& line);
    void emitLabel(const std::string& label);

    int maxRegisterArgs() const;
    const char* argRegister(int index) const;
    int shadowSpaceBytes() const;

    void collectSignatures(const Program* program);
    void checkArity(const std::string& name, size_t argCount, int line, int column) const;

    void genFunction(const FunctionDecl* decl);
    void genParamBinding(const Param& param, int index);

    void pushScope();
    void popScope();
    int declareLocal(const std::string& name, int line, int column);
    int lookupLocal(const std::string& name, int line, int column) const;

    void genStatements(const std::vector<std::unique_ptr<Stmt>>& stmts);
    void genStmt(const Stmt* stmt);
    void genVarDeclStmt(const VarDeclStmt* stmt);
    void genReturnStmt(const ReturnStmt* stmt);
    void genIfStmt(const IfStmt* stmt);
    void genForStmt(const ForStmt* stmt);
    void genAsmStmt(const AsmStmt* stmt);
    void genExprStmt(const ExprStmt* stmt);

    void genExpr(const Expr* expr);
    void genBinaryExpr(const BinaryExpr* expr);
    void genUnaryExpr(const UnaryExpr* expr);
    void genCallExpr(const CallExpr* expr);
    void genAssignExpr(const AssignExpr* expr);
    void materializeBool(const Expr* expr);

    void genJumpIfTrue(const Expr* expr, const std::string& label);
    void genJumpIfFalse(const Expr* expr, const std::string& label);
    void genComparisonJump(const BinaryExpr* expr, const std::string& label, bool jumpWhenTrue);

    std::string addStringConstant(const std::string& value);
    bool isSupportedType(const Type* type) const;
    std::string describeUnsupportedType(const Type* type) const;

    [[noreturn]] void error(const std::string& message, int line, int column) const;
};