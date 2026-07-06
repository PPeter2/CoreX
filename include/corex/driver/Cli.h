#pragma once
#include <string>
#include <vector>

class Cli {
public:
    static int run(int argc, char** argv);

private:
    static int commandBuild(const std::vector<std::string>& args);
    static int commandRun(const std::vector<std::string>& args);
    static int commandTokens(const std::vector<std::string>& args);
    static int commandExpr(const std::vector<std::string>& args);
    static int commandType(const std::vector<std::string>& args);
    static int commandStmt(const std::vector<std::string>& args);
    static int commandParse(const std::vector<std::string>& args);
    static int commandCheck(const std::vector<std::string>& args);
    static int commandInstall(const std::vector<std::string>& args);
    static int commandVersion(const std::vector<std::string>& args);
    static int commandHelp(const std::vector<std::string>& args);

    static void printUsage();
};