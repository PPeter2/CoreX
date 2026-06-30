#include "corex/parser/Parser.h"
#include <stdexcept>
#include <sstream>

Parser::Parser(std::vector<Token> tokens)
    : tokens(std::move(tokens)), pos(0) {}

const Token& Parser::peek() const {
    return tokens[pos];
}

const Token& Parser::peekNext() const {
    if (pos + 1 >= tokens.size()) return tokens[tokens.size() - 1];
    return tokens[pos + 1];
}

const Token& Parser::advance() {
    const Token& current = tokens[pos];
    if (!isAtEnd()) pos++;
    return current;
}

bool Parser::check(TokenType type) const {
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

const Token& Parser::expect(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    error(message);
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}

void Parser::error(const std::string& message) {
    std::ostringstream oss;
    oss << "Parser error at " << peek().line << ":" << peek().column << " - " << message << " (got '" << peek().lexeme << "')";
    throw std::runtime_error(oss.str());
}

bool Parser::isAssignmentOperator() const {
    switch (peek().type) {
        case TokenType::ASSIGN:
        case TokenType::PLUS_ASSIGN:
        case TokenType::MINUS_ASSIGN:
        case TokenType::STAR_ASSIGN:
        case TokenType::SLASH_ASSIGN:
        case TokenType::PERCENT_ASSIGN:
            return true;
        default:
            return false;
    }
}

std::unique_ptr<Expr> Parser::parseExpression() {
    return parseAssignment();
}

std::unique_ptr<Expr> Parser::parseAssignment() {
    std::unique_ptr<Expr> left = parseLogicalOr();

    if (isAssignmentOperator()) {
        const Token& opToken = advance();
        std::unique_ptr<Expr> right = parseAssignment();
        return std::make_unique<AssignExpr>(opToken.lexeme, std::move(left), std::move(right), opToken.line, opToken.column);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseLogicalOr() {
    std::unique_ptr<Expr> left = parseLogicalAnd();

    while (check(TokenType::OR_OR)) {
        const Token& opToken = advance();
        std::unique_ptr<Expr> right = parseLogicalAnd();
        left = std::make_unique<BinaryExpr>("||", std::move(left), std::move(right), opToken.line, opToken.column);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseLogicalAnd() {
    std::unique_ptr<Expr> left = parseEquality();

    while (check(TokenType::AND_AND)) {
        const Token& opToken = advance();
        std::unique_ptr<Expr> right = parseEquality();
        left = std::make_unique<BinaryExpr>("&&", std::move(left), std::move(right), opToken.line, opToken.column);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseEquality() {
    std::unique_ptr<Expr> left = parseComparison();

    while (check(TokenType::EQ) || check(TokenType::NEQ)) {
        const Token& opToken = advance();
        std::unique_ptr<Expr> right = parseComparison();
        left = std::make_unique<BinaryExpr>(opToken.lexeme, std::move(left), std::move(right), opToken.line, opToken.column);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseComparison() {
    std::unique_ptr<Expr> left = parseRange();

    while (check(TokenType::LT) || check(TokenType::GT) || check(TokenType::LE) || check(TokenType::GE)) {
        const Token& opToken = advance();
        std::unique_ptr<Expr> right = parseRange();
        left = std::make_unique<BinaryExpr>(opToken.lexeme, std::move(left), std::move(right), opToken.line, opToken.column);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseRange() {
    std::unique_ptr<Expr> left = parseAdditive();

    if (check(TokenType::RANGE) || check(TokenType::RANGE_INCLUSIVE)) {
        bool inclusive = check(TokenType::RANGE_INCLUSIVE);
        const Token& opToken = advance();
        std::unique_ptr<Expr> right = parseAdditive();
        return std::make_unique<RangeExpr>(std::move(left), std::move(right), inclusive, opToken.line, opToken.column);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseAdditive() {
    std::unique_ptr<Expr> left = parseMultiplicative();

    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        const Token& opToken = advance();
        std::unique_ptr<Expr> right = parseMultiplicative();
        left = std::make_unique<BinaryExpr>(opToken.lexeme, std::move(left), std::move(right), opToken.line, opToken.column);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseMultiplicative() {
    std::unique_ptr<Expr> left = parseUnary();

    while (check(TokenType::STAR) || check(TokenType::SLASH) || check(TokenType::PERCENT)) {
        const Token& opToken = advance();
        std::unique_ptr<Expr> right = parseUnary();
        left = std::make_unique<BinaryExpr>(opToken.lexeme, std::move(left), std::move(right), opToken.line, opToken.column);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (check(TokenType::MINUS) || check(TokenType::BANG) || check(TokenType::STAR) || check(TokenType::AMP)) {
        const Token& opToken = advance();
        std::unique_ptr<Expr> operand = parseUnary();
        return std::make_unique<UnaryExpr>(opToken.lexeme, std::move(operand), opToken.line, opToken.column);
    }

    return parsePostfix();
}

std::unique_ptr<Expr> Parser::parsePostfix() {
    std::unique_ptr<Expr> expr = parsePrimary();

    while (true) {
        if (check(TokenType::LPAREN)) {
            const Token& openToken = advance();
            std::vector<std::unique_ptr<Expr>> arguments = parseArgumentList();
            expect(TokenType::RPAREN, "expected ')' after call arguments");
            expr = std::make_unique<CallExpr>(std::move(expr), std::move(arguments), openToken.line, openToken.column);
        } else if (check(TokenType::LBRACKET)) {
            const Token& openToken = advance();
            std::unique_ptr<Expr> index = parseExpression();
            expect(TokenType::RBRACKET, "expected ']' after index expression");
            expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index), openToken.line, openToken.column);
        } else {
            break;
        }
    }

    return expr;
}

std::vector<std::unique_ptr<Expr>> Parser::parseArgumentList() {
    std::vector<std::unique_ptr<Expr>> arguments;

    if (check(TokenType::RPAREN)) {
        return arguments;
    }

    arguments.push_back(parseExpression());

    while (match(TokenType::COMMA)) {
        arguments.push_back(parseExpression());
    }

    return arguments;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    const Token& token = peek();

    if (check(TokenType::INT_LITERAL)) {
        advance();
        return std::make_unique<IntLiteralExpr>(token.lexeme, token.line, token.column);
    }

    if (check(TokenType::FLOAT_LITERAL)) {
        advance();
        return std::make_unique<FloatLiteralExpr>(token.lexeme, token.line, token.column);
    }

    if (check(TokenType::STRING_LITERAL)) {
        advance();
        return std::make_unique<StringLiteralExpr>(token.lexeme, token.line, token.column);
    }

    if (check(TokenType::CHAR_LITERAL)) {
        advance();
        return std::make_unique<CharLiteralExpr>(token.lexeme[0], token.line, token.column);
    }

    if (check(TokenType::TRUE)) {
        advance();
        return std::make_unique<BoolLiteralExpr>(true, token.line, token.column);
    }

    if (check(TokenType::FALSE)) {
        advance();
        return std::make_unique<BoolLiteralExpr>(false, token.line, token.column);
    }

    if (check(TokenType::IDENTIFIER)) {
        advance();
        return std::make_unique<IdentifierExpr>(token.lexeme, token.line, token.column);
    }

    if (check(TokenType::LPAREN)) {
        advance();
        std::unique_ptr<Expr> expr = parseExpression();
        expect(TokenType::RPAREN, "expected ')' after expression");
        return expr;
    }

    error("expected an expression");
}

bool Parser::checkTypeKeyword() const {
    switch (peek().type) {
        case TokenType::TYPE_INT:
        case TokenType::TYPE_I8:
        case TokenType::TYPE_I16:
        case TokenType::TYPE_I32:
        case TokenType::TYPE_I64:
        case TokenType::TYPE_UINT:
        case TokenType::TYPE_U8:
        case TokenType::TYPE_U16:
        case TokenType::TYPE_U32:
        case TokenType::TYPE_U64:
        case TokenType::TYPE_FLOAT:
        case TokenType::TYPE_F32:
        case TokenType::TYPE_F64:
        case TokenType::TYPE_CHAR:
        case TokenType::TYPE_BOOLEAN:
        case TokenType::TYPE_STRING:
        case TokenType::TYPE_VOID:
            return true;
        default:
            return false;
    }
}

std::string Parser::typeKeywordName(TokenType type) const {
    switch (type) {
        case TokenType::TYPE_INT: return "int";
        case TokenType::TYPE_I8: return "i8";
        case TokenType::TYPE_I16: return "i16";
        case TokenType::TYPE_I32: return "i32";
        case TokenType::TYPE_I64: return "i64";
        case TokenType::TYPE_UINT: return "uint";
        case TokenType::TYPE_U8: return "u8";
        case TokenType::TYPE_U16: return "u16";
        case TokenType::TYPE_U32: return "u32";
        case TokenType::TYPE_U64: return "u64";
        case TokenType::TYPE_FLOAT: return "float";
        case TokenType::TYPE_F32: return "f32";
        case TokenType::TYPE_F64: return "f64";
        case TokenType::TYPE_CHAR: return "char";
        case TokenType::TYPE_BOOLEAN: return "boolean";
        case TokenType::TYPE_STRING: return "string";
        case TokenType::TYPE_VOID: return "void";
        default: return "unknown";
    }
}

std::unique_ptr<Type> Parser::parseType() {
    const Token& token = peek();

    if (check(TokenType::STAR)) {
        advance();
        bool isVolatile = false;
        bool isMutable = false;

        if (check(TokenType::KW_VOLATILE)) {
            advance();
            isVolatile = true;
        }

        if (check(TokenType::MUT)) {
            advance();
            isMutable = true;
        } else if (check(TokenType::CONST)) {
            advance();
            isMutable = false;
        } else {
            error("expected 'mut' or 'const' after '*' in pointer type");
        }

        std::unique_ptr<Type> pointee = parseType();
        return std::make_unique<PointerType>(std::move(pointee), isMutable, isVolatile, token.line, token.column);
    }

    if (check(TokenType::FUNC)) {
        advance();
        expect(TokenType::LPAREN, "expected '(' after 'func' in function type");

        std::vector<std::unique_ptr<Type>> paramTypes;

        if (!check(TokenType::RPAREN)) {
            paramTypes.push_back(parseType());

            while (match(TokenType::COMMA)) {
                paramTypes.push_back(parseType());
            }
        }

        expect(TokenType::RPAREN, "expected ')' after function type parameters");

        std::unique_ptr<Type> returnType = nullptr;
        if (match(TokenType::ARROW)) {
            returnType = parseType();
        }

        return std::make_unique<FunctionType>(std::move(paramTypes), std::move(returnType), token.line, token.column);
    }

    if (check(TokenType::LBRACKET)) {
        advance();

        if (check(TokenType::RBRACKET)) {
            advance();
            std::unique_ptr<Type> elementType = parseType();
            return std::make_unique<SliceType>(std::move(elementType), token.line, token.column);
        }

        std::unique_ptr<Type> elementType = parseType();
        expect(TokenType::SEMICOLON, "expected ';' in array type");
        const Token& sizeToken = expect(TokenType::INT_LITERAL, "expected array size");
        expect(TokenType::RBRACKET, "expected ']' after array type");
        return std::make_unique<ArrayType>(std::move(elementType), sizeToken.lexeme, token.line, token.column);
    }

    if (checkTypeKeyword()) {
        std::string name = typeKeywordName(token.type);
        advance();
        return std::make_unique<NamedType>(name, token.line, token.column);
    }

    if (check(TokenType::IDENTIFIER)) {
        std::string name = token.lexeme;
        advance();

        if (check(TokenType::LT)) {
            advance();
            std::vector<std::unique_ptr<Type>> arguments;
            arguments.push_back(parseType());

            while (match(TokenType::COMMA)) {
                arguments.push_back(parseType());
            }

            expect(TokenType::GT, "expected '>' after generic type arguments");
            return std::make_unique<GenericType>(name, std::move(arguments), token.line, token.column);
        }

        return std::make_unique<NamedType>(name, token.line, token.column);
    }

    error("expected a type");
}

std::unique_ptr<BlockStmt> Parser::parseBlock() {
    const Token& openToken = expect(TokenType::LBRACE, "expected '{' to start block");
    std::vector<std::unique_ptr<Stmt>> statements;

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        statements.push_back(parseStatement());
    }

    expect(TokenType::RBRACE, "expected '}' to close block");
    return std::make_unique<BlockStmt>(std::move(statements), openToken.line, openToken.column);
}

std::unique_ptr<Stmt> Parser::parseVarDeclStmt(bool isMutable) {
    const Token& keywordToken = tokens[pos - 1];

    const Token& nameToken = expect(TokenType::IDENTIFIER, "expected a variable name");
    std::unique_ptr<Type> type = nullptr;

    if (match(TokenType::COLON)) {
        type = parseType();
    }

    std::unique_ptr<Expr> initializer = nullptr;

    if (match(TokenType::ASSIGN)) {
        initializer = parseExpression();
    }

    if (type == nullptr && initializer == nullptr) {
        error("variable declaration needs a type, an initializer, or both");
    }

    return std::make_unique<VarDeclStmt>(nameToken.lexeme, isMutable, std::move(type), std::move(initializer), keywordToken.line, keywordToken.column);
}

std::unique_ptr<Stmt> Parser::parseReturnStmt() {
    const Token& keywordToken = expect(TokenType::RETURN, "expected 'return'");
    std::vector<std::unique_ptr<Expr>> values;

    if (check(TokenType::LPAREN) && peekNext().type != TokenType::RPAREN) {
        size_t savedPos = pos;
        advance();
        std::unique_ptr<Expr> first = parseExpression();

        if (check(TokenType::COMMA)) {
            values.push_back(std::move(first));

            while (match(TokenType::COMMA)) {
                values.push_back(parseExpression());
            }

            expect(TokenType::RPAREN, "expected ')' after return values");
            return std::make_unique<ReturnStmt>(std::move(values), keywordToken.line, keywordToken.column);
        }

        pos = savedPos;
    }

    if (!check(TokenType::RBRACE)) {
        values.push_back(parseExpression());
    }

    return std::make_unique<ReturnStmt>(std::move(values), keywordToken.line, keywordToken.column);
}

std::unique_ptr<Stmt> Parser::parseDeferStmt() {
    const Token& keywordToken = expect(TokenType::DEFER, "expected 'defer'");
    std::unique_ptr<Expr> expression = parseExpression();
    return std::make_unique<DeferStmt>(std::move(expression), keywordToken.line, keywordToken.column);
}

std::unique_ptr<Stmt> Parser::parseIfStmt() {
    const Token& keywordToken = expect(TokenType::IF, "expected 'if'");
    std::unique_ptr<Expr> condition = parseExpression();
    std::unique_ptr<BlockStmt> thenBranch = parseBlock();
    std::unique_ptr<Stmt> elseBranch = nullptr;

    if (match(TokenType::ELSE)) {
        if (check(TokenType::IF)) {
            elseBranch = parseIfStmt();
        } else {
            elseBranch = parseBlock();
        }
    }

    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch), keywordToken.line, keywordToken.column);
}

std::unique_ptr<Stmt> Parser::parseForStmt() {
    const Token& keywordToken = expect(TokenType::FOR, "expected 'for'");

    std::unique_ptr<Stmt> initializer = nullptr;

    if (check(TokenType::LET)) {
        advance();
        initializer = parseVarDeclStmt(false);
    } else if (check(TokenType::MUT)) {
        advance();
        initializer = parseVarDeclStmt(true);
    } else if (!check(TokenType::SEMICOLON)) {
        initializer = std::make_unique<ExprStmt>(parseExpression(), peek().line, peek().column);
    }

    expect(TokenType::SEMICOLON, "expected ';' after for-loop initializer");

    std::unique_ptr<Expr> condition = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        condition = parseExpression();
    }

    expect(TokenType::SEMICOLON, "expected ';' after for-loop condition");

    std::unique_ptr<Stmt> increment = nullptr;
    if (!check(TokenType::LBRACE)) {
        increment = std::make_unique<ExprStmt>(parseExpression(), peek().line, peek().column);
    }

    std::unique_ptr<BlockStmt> body = parseBlock();

    return std::make_unique<ForStmt>(std::move(initializer), std::move(condition), std::move(increment), std::move(body), keywordToken.line, keywordToken.column);
}

std::unique_ptr<Stmt> Parser::parseAsmStmt() {
    const Token& keywordToken = expect(TokenType::ASM, "expected 'asm'");
    expect(TokenType::LBRACE, "expected '{' after 'asm'");

    std::vector<std::string> instructions;

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        const Token& instructionToken = expect(TokenType::STRING_LITERAL, "expected a string literal instruction inside asm block");
        instructions.push_back(instructionToken.lexeme);
    }

    expect(TokenType::RBRACE, "expected '}' to close asm block");
    return std::make_unique<AsmStmt>(std::move(instructions), keywordToken.line, keywordToken.column);
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (check(TokenType::LET)) {
        advance();
        return parseVarDeclStmt(false);
    }

    if (check(TokenType::MUT)) {
        advance();
        return parseVarDeclStmt(true);
    }

    if (check(TokenType::RETURN)) {
        return parseReturnStmt();
    }

    if (check(TokenType::DEFER)) {
        return parseDeferStmt();
    }

    if (check(TokenType::IF)) {
        return parseIfStmt();
    }

    if (check(TokenType::FOR)) {
        return parseForStmt();
    }

    if (check(TokenType::ASM)) {
        return parseAsmStmt();
    }

    if (check(TokenType::LBRACE)) {
        return parseBlock();
    }

    const Token& token = peek();
    std::unique_ptr<Expr> expr = parseExpression();
    return std::make_unique<ExprStmt>(std::move(expr), token.line, token.column);
}

std::vector<std::string> Parser::parseGenericParamList() {
    std::vector<std::string> params;

    if (!match(TokenType::LT)) {
        return params;
    }

    const Token& first = expect(TokenType::IDENTIFIER, "expected a generic parameter name");
    params.push_back(first.lexeme);

    while (match(TokenType::COMMA)) {
        const Token& next = expect(TokenType::IDENTIFIER, "expected a generic parameter name");
        params.push_back(next.lexeme);
    }

    expect(TokenType::GT, "expected '>' after generic parameter list");
    return params;
}

std::vector<Param> Parser::parseParamList() {
    std::vector<Param> params;

    expect(TokenType::LPAREN, "expected '(' to start parameter list");

    if (check(TokenType::RPAREN)) {
        advance();
        return params;
    }

    while (true) {
        const Token& nameToken = expect(TokenType::IDENTIFIER, "expected a parameter name");
        expect(TokenType::COLON, "expected ':' after parameter name");
        std::unique_ptr<Type> type = parseType();
        params.emplace_back(nameToken.lexeme, std::move(type));

        if (!match(TokenType::COMMA)) {
            break;
        }
    }

    expect(TokenType::RPAREN, "expected ')' after parameter list");
    return params;
}

std::unique_ptr<FunctionDecl> Parser::parseFunctionDecl(bool isExternSignatureOnly) {
    const Token& keywordToken = expect(TokenType::FUNC, "expected 'func'");
    const Token& nameToken = expect(TokenType::IDENTIFIER, "expected a function name");

    std::vector<std::string> genericParams = parseGenericParamList();
    std::vector<Param> params = parseParamList();
    std::vector<std::unique_ptr<Type>> returnTypes;

    if (match(TokenType::ARROW)) {
        if (match(TokenType::LPAREN)) {
            returnTypes.push_back(parseType());

            while (match(TokenType::COMMA)) {
                returnTypes.push_back(parseType());
            }

            expect(TokenType::RPAREN, "expected ')' after multiple return types");
        } else {
            returnTypes.push_back(parseType());
        }
    }

    std::unique_ptr<BlockStmt> body = nullptr;

    if (isExternSignatureOnly) {
        body = nullptr;
    } else {
        body = parseBlock();
    }

    return std::make_unique<FunctionDecl>(nameToken.lexeme, std::move(genericParams), std::move(params), std::move(returnTypes), std::move(body), isExternSignatureOnly, keywordToken.line, keywordToken.column);
}

std::unique_ptr<Decl> Parser::parseStructDecl() {
    const Token& keywordToken = expect(TokenType::STRUCT, "expected 'struct'");
    const Token& nameToken = expect(TokenType::IDENTIFIER, "expected a struct name");
    std::vector<std::string> genericParams = parseGenericParamList();

    expect(TokenType::LBRACE, "expected '{' to start struct body");
    std::vector<Field> fields;

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        const Token& fieldNameToken = expect(TokenType::IDENTIFIER, "expected a field name");
        expect(TokenType::COLON, "expected ':' after field name");
        std::unique_ptr<Type> fieldType = parseType();
        fields.emplace_back(fieldNameToken.lexeme, std::move(fieldType));
    }

    expect(TokenType::RBRACE, "expected '}' to close struct body");
    return std::make_unique<StructDecl>(nameToken.lexeme, std::move(genericParams), std::move(fields), keywordToken.line, keywordToken.column);
}

std::unique_ptr<Decl> Parser::parseEnumDecl() {
    const Token& keywordToken = expect(TokenType::ENUM, "expected 'enum'");
    const Token& nameToken = expect(TokenType::IDENTIFIER, "expected an enum name");

    expect(TokenType::LBRACE, "expected '{' to start enum body");
    std::vector<std::string> variants;

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        const Token& variantToken = expect(TokenType::IDENTIFIER, "expected an enum variant name");
        variants.push_back(variantToken.lexeme);
    }

    expect(TokenType::RBRACE, "expected '}' to close enum body");
    return std::make_unique<EnumDecl>(nameToken.lexeme, std::move(variants), keywordToken.line, keywordToken.column);
}

std::unique_ptr<Decl> Parser::parseExternBlockDecl() {
    const Token& keywordToken = expect(TokenType::EXTERN, "expected 'extern'");
    const Token& abiToken = expect(TokenType::STRING_LITERAL, "expected an ABI name string after 'extern'");

    expect(TokenType::LBRACE, "expected '{' to start extern block");
    std::vector<std::unique_ptr<FunctionDecl>> functions;

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        functions.push_back(parseFunctionDecl(true));
    }

    expect(TokenType::RBRACE, "expected '}' to close extern block");
    return std::make_unique<ExternBlockDecl>(abiToken.lexeme, std::move(functions), keywordToken.line, keywordToken.column);
}

std::unique_ptr<Decl> Parser::parseUsingDecl() {
    const Token& keywordToken = expect(TokenType::USING, "expected 'using'");
    const Token& pathToken = expect(TokenType::STRING_LITERAL, "expected a module path string after 'using'");
    return std::make_unique<UsingDecl>(pathToken.lexeme, keywordToken.line, keywordToken.column);
}

std::vector<std::string> Parser::parseAttributes() {
    std::vector<std::string> attributes;

    while (check(TokenType::ATTRIBUTE)) {
        attributes.push_back(advance().lexeme);
    }

    return attributes;
}

std::unique_ptr<Decl> Parser::parseDeclaration() {
    std::vector<std::string> attributes = parseAttributes();
    std::unique_ptr<Decl> decl;

    if (check(TokenType::FUNC)) {
        decl = parseFunctionDecl(false);
    } else if (check(TokenType::STRUCT)) {
        decl = parseStructDecl();
    } else if (check(TokenType::ENUM)) {
        decl = parseEnumDecl();
    } else if (check(TokenType::EXTERN)) {
        decl = parseExternBlockDecl();
    } else if (check(TokenType::USING)) {
        decl = parseUsingDecl();
    } else if (check(TokenType::LET)) {
        const Token& keywordToken = advance();
        const Token& nameToken = expect(TokenType::IDENTIFIER, "expected a variable name");
        std::unique_ptr<Type> type = nullptr;
        if (match(TokenType::COLON)) {
            type = parseType();
        }
        std::unique_ptr<Expr> initializer = nullptr;
        if (match(TokenType::ASSIGN)) {
            initializer = parseExpression();
        }
        if (type == nullptr && initializer == nullptr) {
            error("global variable declaration needs a type, an initializer, or both");
        }
        decl = std::make_unique<GlobalVarDecl>(nameToken.lexeme, false, std::move(type), std::move(initializer), keywordToken.line, keywordToken.column);
    } else if (check(TokenType::MUT)) {
        const Token& keywordToken = advance();
        const Token& nameToken = expect(TokenType::IDENTIFIER, "expected a variable name");
        std::unique_ptr<Type> type = nullptr;
        if (match(TokenType::COLON)) {
            type = parseType();
        }
        std::unique_ptr<Expr> initializer = nullptr;
        if (match(TokenType::ASSIGN)) {
            initializer = parseExpression();
        }
        if (type == nullptr && initializer == nullptr) {
            error("global variable declaration needs a type, an initializer, or both");
        }
        decl = std::make_unique<GlobalVarDecl>(nameToken.lexeme, true, std::move(type), std::move(initializer), keywordToken.line, keywordToken.column);
    } else {
        error("expected a top-level declaration (func, struct, enum, extern, using, let, or mut)");
    }

    decl->attributes = std::move(attributes);
    return decl;
}

std::unique_ptr<Program> Parser::parseProgram() {
    std::vector<std::unique_ptr<Decl>> declarations;

    while (!isAtEnd()) {
        declarations.push_back(parseDeclaration());
    }

    return std::make_unique<Program>(std::move(declarations));
}