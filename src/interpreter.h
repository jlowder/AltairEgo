#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "lexer.h"
#include "parser.h"
#include "variable.h"
#include "functions.h"
#include <map>
#include <stack>
#include <vector>
#include <memory>

#define DEBUG_PRINT(x) do { if (debug) { std::cout << "[DEBUG] " << x << std::endl; } } while (0)

struct ProgramLine {
    int lineNumber;
    std::shared_ptr<ASTNode> ast;
    
    ProgramLine(int num, std::shared_ptr<ASTNode> node) : lineNumber(num), ast(node) {}
};

struct ForLoopState {
    std::string variable;
    double endValue;
    double stepValue;
    int returnLine;
    int returnStatementIndex;
    
    ForLoopState(const std::string& var, double end, double step, int line, int stmtIndex = -1)
        : variable(var), endValue(end), stepValue(step), returnLine(line), returnStatementIndex(stmtIndex) {}
};

struct CallFrame {
    int returnLine;
    int returnStatementIndex;
    
    CallFrame(int line, int stmtIdx) : returnLine(line), returnStatementIndex(stmtIdx) {}
};

struct UserDefinedFunction {
    std::string name;
    std::string parameter;
    std::shared_ptr<ASTNode> body;
    
    UserDefinedFunction() = default;
    UserDefinedFunction(const std::string& n, const std::string& p, std::shared_ptr<ASTNode> b)
        : name(n), parameter(p), body(b) {}
};

class AltairBasicInterpreter {
private:
    Lexer lexer;
    Parser parser;
    VariableManager variables;
    
    std::map<int, ProgramLine> program;
    std::vector<std::string> dataItems;
    size_t dataPointer;
    std::map<std::string, UserDefinedFunction> userDefinedFunctions;
    
    std::stack<CallFrame> callStack;
    std::stack<ForLoopState> forLoopStack;
    
    int currentLine;
    int currentStatementIndex;
    bool running;
    bool stopExecution;
    bool returningFromSubroutine;
    bool debug;
    int m_currentColumn;
    
    // Execution methods
    void executeProgram();
    void executeLine(std::shared_ptr<ASTNode> line);
    void executeStatement(std::shared_ptr<ASTNode> stmt);
    double evaluateExpression(std::shared_ptr<ASTNode> expr);
    std::string evaluateStringExpression(std::shared_ptr<ASTNode> expr);
    int getNextLineNumber(int currentLineNum);
    void findMatchingNext(int forLineNum);
    void gotoStatement(int lineNum, int statementIndex);
    void cleanupForLoopStackOnGoto(int fromLine, int toLine);
    
    // Statement execution methods
    void executePrint(std::shared_ptr<ASTNode> stmt);
    void executeInput(std::shared_ptr<ASTNode> stmt);
    void executeLet(std::shared_ptr<ASTNode> stmt);
    void executeIf(std::shared_ptr<ASTNode> stmt);
    void executeFor(std::shared_ptr<ASTNode> stmt);
    void executeNext(std::shared_ptr<ASTNode> stmt);
    void executeGoto(std::shared_ptr<ASTNode> stmt);
    void executeGosub(std::shared_ptr<ASTNode> stmt);
    void executeReturn(std::shared_ptr<ASTNode> stmt);
    void executeData(std::shared_ptr<ASTNode> stmt);
    void executeRead(std::shared_ptr<ASTNode> stmt);
    void executeRestore(std::shared_ptr<ASTNode> stmt);
    void executeEnd(std::shared_ptr<ASTNode> stmt);
    void executeStop(std::shared_ptr<ASTNode> stmt);
    void executeOn(std::shared_ptr<ASTNode> stmt);
    void executeDim(std::shared_ptr<ASTNode> stmt);
    void executeDef(std::shared_ptr<ASTNode> stmt);
    
    // Command execution methods
    void executeList();
    void executeNew();
    void executeClear();
    
    // Utility methods
    bool isDirectMode(std::shared_ptr<ASTNode> line);
    bool isCommand(std::shared_ptr<ASTNode> stmt);
    void gotoLine(int lineNumber);
    void collectDataItems();
    std::string formatNumber(double value);
    void printTabs(int count);
    void printStatement(std::shared_ptr<ASTNode> stmt);
    
public:
    AltairBasicInterpreter();
    void processLine(const std::string& input);
    void executeRun();
};

#endif
