#include "corex/driver/Cli.h"
#include "corex/lexer/Lexer.h"
#include "corex/parser/Parser.h"
#include "corex/ast/AstPrinter.h"
#include "corex/ast/TypePrinter.h"
#include "corex/ast/StmtPrinter.h"
#include "corex/ast/DeclPrinter.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace {

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("could not open file: " + path);
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<Token> lexFile(const std::string& path) {
    std::string source = readFile(path);
    Lexer lexer(source);
    return lexer.tokenize();
}

}

int Cli::run(int argc, char** argv) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string command = argv[1];
    std::vector<std::string> args;
    for (int i = 2; i < argc; i++) {
        args.emplace_back(argv[i]);
    }

    if (command == "build") return commandBuild(args);
    if (command == "run") return commandRun(args);
    if (command == "tokens") return commandTokens(args);
    if (command == "expr") return commandExpr(args);
    if (command == "type") return commandType(args);
    if (command == "stmt") return commandStmt(args);
    if (command == "parse") return commandParse(args);
    if (command == "install") return commandInstall(args);
    if (command == "version") return commandVersion(args);
    if (command == "--version") return commandVersion(args);
    if (command == "help") return commandHelp(args);
    if (command == "--help") return commandHelp(args);

    std::cerr << "corex: unknown command '" << command << "'" << std::endl;
    printUsage();
    return 1;
}

int Cli::commandBuild(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "corex build: expected a source file" << std::endl;
        return 1;
    }

    try {
        std::vector<Token> tokens = lexFile(args[0]);
        Parser parser(tokens);
        std::unique_ptr<Program> program = parser.parseProgram();
        std::cout << "corex build: parsed " << program->declarations.size() << " top-level declarations from " << args[0] << std::endl;
        std::cout << "corex build: type checking and code generation are not implemented yet" << std::endl;
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "corex build: " << ex.what() << std::endl;
        return 1;
    }
}

int Cli::commandRun(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "corex run: expected a source file" << std::endl;
        return 1;
    }

    try {
        std::vector<Token> tokens = lexFile(args[0]);
        Parser parser(tokens);
        std::unique_ptr<Program> program = parser.parseProgram();
        std::cout << "corex run: parsed " << program->declarations.size() << " top-level declarations from " << args[0] << std::endl;
        std::cout << "corex run: cannot execute yet, the compiler pipeline is incomplete" << std::endl;
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "corex run: " << ex.what() << std::endl;
        return 1;
    }
}

int Cli::commandTokens(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "corex tokens: expected a source file" << std::endl;
        return 1;
    }

    try {
        std::vector<Token> tokens = lexFile(args[0]);
        for (const Token& token : tokens) {
            std::cout << token.toString() << std::endl;
        }
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "corex tokens: " << ex.what() << std::endl;
        return 1;
    }
}

int Cli::commandInstall(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "corex install: expected a package name, e.g. 'corex install std'" << std::endl;
        return 1;
    }

    std::cout << "corex install: package management is not implemented yet" << std::endl;
    std::cout << "corex install: requested package '" << args[0] << "'" << std::endl;
    return 0;
}

int Cli::commandExpr(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "corex expr: expected an expression string, e.g. 'corex expr \"1 + 2 * 3\"'" << std::endl;
        return 1;
    }

    try {
        Lexer lexer(args[0]);
        std::vector<Token> tokens = lexer.tokenize();
        Parser parser(tokens);
        std::unique_ptr<Expr> expr = parser.parseExpression();
        std::cout << exprToString(expr.get()) << std::endl;
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "corex expr: " << ex.what() << std::endl;
        return 1;
    }
}

int Cli::commandType(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "corex type: expected a type string, e.g. 'corex type \"*mut int\"'" << std::endl;
        return 1;
    }

    try {
        Lexer lexer(args[0]);
        std::vector<Token> tokens = lexer.tokenize();
        Parser parser(tokens);
        std::unique_ptr<Type> type = parser.parseType();
        std::cout << typeToString(type.get()) << std::endl;
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "corex type: " << ex.what() << std::endl;
        return 1;
    }
}

int Cli::commandStmt(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "corex stmt: expected a statement string, e.g. 'corex stmt \"let x: int = 10\"'" << std::endl;
        return 1;
    }

    try {
        Lexer lexer(args[0]);
        std::vector<Token> tokens = lexer.tokenize();
        Parser parser(tokens);
        std::unique_ptr<Stmt> stmt = parser.parseStatement();
        std::cout << stmtToString(stmt.get(), 0) << std::endl;
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "corex stmt: " << ex.what() << std::endl;
        return 1;
    }
}

int Cli::commandParse(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "corex parse: expected a source file" << std::endl;
        return 1;
    }

    try {
        std::vector<Token> tokens = lexFile(args[0]);
        Parser parser(tokens);
        std::unique_ptr<Program> program = parser.parseProgram();
        std::cout << programToString(program.get()) << std::endl;
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "corex parse: " << ex.what() << std::endl;
        return 1;
    }
}

int Cli::commandVersion(const std::vector<std::string>&) {
    std::cout << "corex 0.1.0" << std::endl;
    return 0;
}

int Cli::commandHelp(const std::vector<std::string>&) {
    printUsage();
    return 0;
}

void Cli::printUsage() {
    std::cout << "corex - the CoreX language toolchain" << std::endl;
    std::cout << std::endl;
    std::cout << "usage:" << std::endl;
    std::cout << "  corex build <file.cx>     compile a CoreX source file" << std::endl;
    std::cout << "  corex run <file.cx>       compile and run a CoreX source file" << std::endl;
    std::cout << "  corex tokens <file.cx>    print the lexer token stream for a file" << std::endl;
    std::cout << "  corex expr <expression>  parse a single expression and print its AST" << std::endl;
    std::cout << "  corex type <type>        parse a single type and print its AST" << std::endl;
    std::cout << "  corex stmt <statement>   parse a single statement and print its AST" << std::endl;
    std::cout << "  corex parse <file.cx>    parse an entire file and print the full AST" << std::endl;
    std::cout << "  corex install <package>   install a CoreX package" << std::endl;
    std::cout << "  corex version             print the installed corex version" << std::endl;
    std::cout << "  corex help                show this message" << std::endl;
}