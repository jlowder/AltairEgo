#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <vector>
#include <memory>

// AST Node types
enum NodeType {
    NODE_PROGRAM,
    NODE_LINE,
    NODE_STATEMENT,
    NODE_EXPRESSION,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_NUMBER,
    NODE_VARIABLE,
    NODE_STRING,
    NODE_FUNCTION_CALL,
    NODE_STRING_FUNCTION_CALL,
    NODE_ARRAY_ACCESS,
    NODE_DIM_DECLARATION
};

struct ASTNode {
    NodeType type;
    std::string value;
    std::vector<std::shared_ptr<ASTNode>> children;
    KeywordType keyword;
    OperatorType operator_type;
    int line_number;
    
    ASTNode(NodeType t = NODE_EXPRESSION, const std::string& v = "") 
        : type(t), value(v), keyword(KW_PRINT), operator_type(OP_PLUS), line_number(0) {}
};

class Parser {
private:
    std::vector<Token> tokens;
    size_t current;
    
    Token getCurrentToken();
    Token peekToken();
    void advance();
    bool match(TokenType type);
    bool matchKeyword(KeywordType keyword);
    bool matchOperator(OperatorType op);
    
    std::shared_ptr<ASTNode> parseProgram();
    std::shared_ptr<ASTNode> parseLine();
    std::shared_ptr<ASTNode> parseStatement();
    std::shared_ptr<ASTNode> parseExpression();
    std::shared_ptr<ASTNode> parseLogicalOr();
    std::shared_ptr<ASTNode> parseLogicalAnd();
    std::shared_ptr<ASTNode> parseRelational();
    std::shared_ptr<ASTNode> parseArithmetic();
    std::shared_ptr<ASTNode> parseTerm();
    std::shared_ptr<ASTNode> parseFactor();
    std::shared_ptr<ASTNode> parsePrimary();
    std::shared_ptr<ASTNode> parseVariableList();
    std::shared_ptr<ASTNode> parseExpressionList();
    
    std::shared_ptr<ASTNode> parsePrintStatement();
    std::shared_ptr<ASTNode> parseInputStatement();
    std::shared_ptr<ASTNode> parseLetStatement();
    std::shared_ptr<ASTNode> parseIfStatement();
    std::shared_ptr<ASTNode> parseForStatement();
    std::shared_ptr<ASTNode> parseGotoStatement();
    std::shared_ptr<ASTNode> parseGosubStatement();
    std::shared_ptr<ASTNode> parseReturnStatement();
    std::shared_ptr<ASTNode> parseRemStatement();
    std::shared_ptr<ASTNode> parseDataStatement();
    std::shared_ptr<ASTNode> parseReadStatement();
    std::shared_ptr<ASTNode> parseRestoreStatement();
    std::shared_ptr<ASTNode> parseEndStatement();
    std::shared_ptr<ASTNode> parseStopStatement();
    std::shared_ptr<ASTNode> parseNextStatement();
    std::shared_ptr<ASTNode> parseOnStatement();
    std::shared_ptr<ASTNode> parseListStatement();
    std::shared_ptr<ASTNode> parseNewStatement();
    std::shared_ptr<ASTNode> parseRunStatement();
    std::shared_ptr<ASTNode> parseClearStatement();
    std::shared_ptr<ASTNode> parseDimStatement();
    std::shared_ptr<ASTNode> parseDefStatement();
    
public:
    Parser();
    std::shared_ptr<ASTNode> parse(const std::vector<Token>& tokenList);
};

#endif
