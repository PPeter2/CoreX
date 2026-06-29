#include "corex/lexer/Lexer.h"
#include <cctype>
#include <stdexcept>
#include <sstream>

const std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"let", TokenType::LET},
    {"mut", TokenType::MUT},
    {"const", TokenType::CONST},
    {"struct", TokenType::STRUCT},
    {"enum", TokenType::ENUM},
    {"union", TokenType::UNION},
    {"func", TokenType::FUNC},
    {"using", TokenType::USING},
    {"extern", TokenType::EXTERN},
    {"impl", TokenType::IMPL},
    {"trait", TokenType::TRAIT},
    {"module", TokenType::MODULE},
    {"pub", TokenType::PUB},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"for", TokenType::FOR},
    {"while", TokenType::WHILE},
    {"loop", TokenType::LOOP},
    {"match", TokenType::MATCH},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"return", TokenType::RETURN},
    {"defer", TokenType::DEFER},
    {"asm", TokenType::ASM},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"null", TokenType::NULLPTR},
    {"int", TokenType::TYPE_INT},
    {"i8", TokenType::TYPE_I8},
    {"i16", TokenType::TYPE_I16},
    {"i32", TokenType::TYPE_I32},
    {"i64", TokenType::TYPE_I64},
    {"uint", TokenType::TYPE_UINT},
    {"u8", TokenType::TYPE_U8},
    {"u16", TokenType::TYPE_U16},
    {"u32", TokenType::TYPE_U32},
    {"u64", TokenType::TYPE_U64},
    {"float", TokenType::TYPE_FLOAT},
    {"f32", TokenType::TYPE_F32},
    {"f64", TokenType::TYPE_F64},
    {"char", TokenType::TYPE_CHAR},
    {"boolean", TokenType::TYPE_BOOLEAN},
    {"string", TokenType::TYPE_STRING},
    {"void", TokenType::TYPE_VOID},
    {"volatile", TokenType::KW_VOLATILE},
    {"unsafe", TokenType::KW_UNSAFE},
    {"sizeof", TokenType::KW_SIZEOF},
    {"alignof", TokenType::KW_ALIGNOF},
    {"as", TokenType::KW_AS},
};

Lexer::Lexer(std::string source)
    : source(std::move(source)), pos(0), line(1), column(1) {}

bool Lexer::isAtEnd() const {
    return pos >= source.size();
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[pos];
}

char Lexer::peekNext() const {
    if (pos + 1 >= source.size()) return '\0';
    return source[pos + 1];
}

char Lexer::advance() {
    char c = source[pos++];
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source[pos] != expected) return false;
    advance();
    return true;
}

void Lexer::addToken(TokenType type, const std::string& lexeme, int startLine, int startColumn) {
    tokens.emplace_back(type, lexeme, startLine, startColumn);
}

void Lexer::error(const std::string& message, int errLine, int errColumn) {
    std::ostringstream oss;
    oss << "Lexer error at " << errLine << ":" << errColumn << " - " << message;
    throw std::runtime_error(oss.str());
}

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else if (c == '/' && peekNext() == '/') {
            while (!isAtEnd() && peek() != '\n') advance();
        } else if (c == '/' && peekNext() == '*') {
            advance();
            advance();
            while (!isAtEnd() && !(peek() == '*' && peekNext() == '/')) {
                advance();
            }
            if (!isAtEnd()) {
                advance();
                advance();
            }
        } else {
            return;
        }
    }
}

void Lexer::scanIdentifierOrKeyword() {
    int startLine = line;
    int startColumn = column;
    size_t start = pos;
    while (!isAtEnd() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) {
        advance();
    }
    std::string text = source.substr(start, pos - start);
    auto it = keywords.find(text);
    if (it != keywords.end()) {
        addToken(it->second, text, startLine, startColumn);
    } else {
        addToken(TokenType::IDENTIFIER, text, startLine, startColumn);
    }
}

void Lexer::scanNumber() {
    int startLine = line;
    int startColumn = column;
    size_t start = pos;
    bool isFloat = false;

    if (peek() == '0' && (peekNext() == 'x' || peekNext() == 'X')) {
        advance();
        advance();
        while (!isAtEnd() && (std::isxdigit(static_cast<unsigned char>(peek())) || peek() == '_')) {
            advance();
        }
        std::string text = source.substr(start, pos - start);
        addToken(TokenType::INT_LITERAL, text, startLine, startColumn);
        return;
    }

    if (peek() == '0' && (peekNext() == 'b' || peekNext() == 'B')) {
        advance();
        advance();
        while (!isAtEnd() && (peek() == '0' || peek() == '1' || peek() == '_')) {
            advance();
        }
        std::string text = source.substr(start, pos - start);
        addToken(TokenType::INT_LITERAL, text, startLine, startColumn);
        return;
    }

    if (peek() == '0' && (peekNext() == 'o' || peekNext() == 'O')) {
        advance();
        advance();
        while (!isAtEnd() && ((peek() >= '0' && peek() <= '7') || peek() == '_')) {
            advance();
        }
        std::string text = source.substr(start, pos - start);
        addToken(TokenType::INT_LITERAL, text, startLine, startColumn);
        return;
    }

    while (!isAtEnd() && (std::isdigit(static_cast<unsigned char>(peek())) || peek() == '_')) {
        advance();
    }
    if (!isAtEnd() && peek() == '.' && std::isdigit(static_cast<unsigned char>(peekNext()))) {
        isFloat = true;
        advance();
        while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
        }
    }
    if (!isAtEnd() && (peek() == 'e' || peek() == 'E')) {
        isFloat = true;
        advance();
        if (!isAtEnd() && (peek() == '+' || peek() == '-')) {
            advance();
        }
        while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
        }
    }
    std::string text = source.substr(start, pos - start);
    addToken(isFloat ? TokenType::FLOAT_LITERAL : TokenType::INT_LITERAL, text, startLine, startColumn);
}

void Lexer::scanString() {
    int startLine = line;
    int startColumn = column;
    advance();
    std::string value;
    while (!isAtEnd() && peek() != '"') {
        char c = peek();
        if (c == '\\') {
            advance();
            if (isAtEnd()) error("unterminated escape sequence in string literal", startLine, startColumn);
            char escaped = advance();
            switch (escaped) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case '0': value += '\0'; break;
                case '"': value += '"'; break;
                case '\\': value += '\\'; break;
                default: value += escaped; break;
            }
        } else {
            value += advance();
        }
    }
    if (isAtEnd()) error("unterminated string literal", startLine, startColumn);
    advance();
    addToken(TokenType::STRING_LITERAL, value, startLine, startColumn);
}

void Lexer::scanChar() {
    int startLine = line;
    int startColumn = column;
    advance();
    if (isAtEnd()) error("unterminated char literal", startLine, startColumn);
    char value;
    if (peek() == '\\') {
        advance();
        if (isAtEnd()) error("unterminated escape sequence in char literal", startLine, startColumn);
        char escaped = advance();
        switch (escaped) {
            case 'n': value = '\n'; break;
            case 't': value = '\t'; break;
            case 'r': value = '\r'; break;
            case '0': value = '\0'; break;
            case '\'': value = '\''; break;
            case '\\': value = '\\'; break;
            default: value = escaped; break;
        }
    } else {
        value = advance();
    }
    if (isAtEnd() || peek() != '\'') error("expected closing quote for char literal", startLine, startColumn);
    advance();
    addToken(TokenType::CHAR_LITERAL, std::string(1, value), startLine, startColumn);
}

void Lexer::scanAttribute() {
    int startLine = line;
    int startColumn = column;
    size_t start = pos;
    advance();
    if (isAtEnd() || peek() != '[') error("expected '[' after '#' to begin attribute", startLine, startColumn);
    advance();
    int depth = 1;
    while (!isAtEnd() && depth > 0) {
        char c = peek();
        if (c == '[') depth++;
        if (c == ']') depth--;
        if (depth > 0) advance();
    }
    if (isAtEnd()) error("unterminated attribute, expected ']'", startLine, startColumn);
    advance();
    std::string text = source.substr(start, pos - start);
    addToken(TokenType::ATTRIBUTE, text, startLine, startColumn);
}

void Lexer::scanToken() {
    int startLine = line;
    int startColumn = column;
    char c = advance();

    switch (c) {
        case '(': addToken(TokenType::LPAREN, "(", startLine, startColumn); return;
        case ')': addToken(TokenType::RPAREN, ")", startLine, startColumn); return;
        case '{': addToken(TokenType::LBRACE, "{", startLine, startColumn); return;
        case '}': addToken(TokenType::RBRACE, "}", startLine, startColumn); return;
        case '[': addToken(TokenType::LBRACKET, "[", startLine, startColumn); return;
        case ']': addToken(TokenType::RBRACKET, "]", startLine, startColumn); return;
        case ',': addToken(TokenType::COMMA, ",", startLine, startColumn); return;
        case ';': addToken(TokenType::SEMICOLON, ";", startLine, startColumn); return;
        case '@': addToken(TokenType::AT, "@", startLine, startColumn); return;
        case '?': addToken(TokenType::QUESTION, "?", startLine, startColumn); return;
        case '~': addToken(TokenType::TILDE, "~", startLine, startColumn); return;

        case ':':
            if (match(':')) addToken(TokenType::COLON_COLON, "::", startLine, startColumn);
            else addToken(TokenType::COLON, ":", startLine, startColumn);
            return;

        case '.':
            if (match('.')) {
                if (match('=')) addToken(TokenType::RANGE_INCLUSIVE, "..=", startLine, startColumn);
                else addToken(TokenType::RANGE, "..", startLine, startColumn);
            } else {
                addToken(TokenType::DOT, ".", startLine, startColumn);
            }
            return;

        case '+':
            if (match('=')) addToken(TokenType::PLUS_ASSIGN, "+=", startLine, startColumn);
            else addToken(TokenType::PLUS, "+", startLine, startColumn);
            return;

        case '-':
            if (match('>')) addToken(TokenType::ARROW, "->", startLine, startColumn);
            else if (match('=')) addToken(TokenType::MINUS_ASSIGN, "-=", startLine, startColumn);
            else addToken(TokenType::MINUS, "-", startLine, startColumn);
            return;

        case '*':
            if (match('=')) addToken(TokenType::STAR_ASSIGN, "*=", startLine, startColumn);
            else addToken(TokenType::STAR, "*", startLine, startColumn);
            return;

        case '/':
            if (match('=')) addToken(TokenType::SLASH_ASSIGN, "/=", startLine, startColumn);
            else addToken(TokenType::SLASH, "/", startLine, startColumn);
            return;

        case '%':
            if (match('=')) addToken(TokenType::PERCENT_ASSIGN, "%=", startLine, startColumn);
            else addToken(TokenType::PERCENT, "%", startLine, startColumn);
            return;

        case '=':
            if (match('=')) addToken(TokenType::EQ, "==", startLine, startColumn);
            else if (match('>')) addToken(TokenType::FAT_ARROW, "=>", startLine, startColumn);
            else addToken(TokenType::ASSIGN, "=", startLine, startColumn);
            return;

        case '!':
            if (match('=')) addToken(TokenType::NEQ, "!=", startLine, startColumn);
            else addToken(TokenType::BANG, "!", startLine, startColumn);
            return;

        case '<':
            if (match('=')) addToken(TokenType::LE, "<=", startLine, startColumn);
            else if (match('<')) addToken(TokenType::SHL, "<<", startLine, startColumn);
            else addToken(TokenType::LT, "<", startLine, startColumn);
            return;

        case '>':
            if (match('=')) addToken(TokenType::GE, ">=", startLine, startColumn);
            else if (match('>')) addToken(TokenType::SHR, ">>", startLine, startColumn);
            else addToken(TokenType::GT, ">", startLine, startColumn);
            return;

        case '&':
            if (match('&')) addToken(TokenType::AND_AND, "&&", startLine, startColumn);
            else addToken(TokenType::AMP, "&", startLine, startColumn);
            return;

        case '|':
            if (match('|')) addToken(TokenType::OR_OR, "||", startLine, startColumn);
            else addToken(TokenType::PIPE, "|", startLine, startColumn);
            return;

        case '^':
            addToken(TokenType::CARET, "^", startLine, startColumn);
            return;

        case '"':
            pos--;
            column--;
            scanString();
            return;

        case '\'':
            pos--;
            column--;
            scanChar();
            return;

        case '#':
            pos--;
            column--;
            scanAttribute();
            return;

        default:
            if (std::isdigit(static_cast<unsigned char>(c))) {
                pos--;
                column--;
                scanNumber();
                return;
            }
            if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                pos--;
                column--;
                scanIdentifierOrKeyword();
                return;
            }
            error("unexpected character", startLine, startColumn);
    }
}

std::vector<Token> Lexer::tokenize() {
    while (true) {
        skipWhitespaceAndComments();
        if (isAtEnd()) break;
        scanToken();
    }
    addToken(TokenType::END_OF_FILE, "", line, column);
    return tokens;
}
