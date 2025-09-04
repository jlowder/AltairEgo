#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <map>

enum TokenType {
    TOKEN_NUMBER,
    TOKEN_VARIABLE,
    TOKEN_STRING,
    TOKEN_KEYWORD,
    TOKEN_OPERATOR,
    TOKEN_DELIMITER,
    TOKEN_NEWLINE,
    TOKEN_EOF
};

enum KeywordType {
    KW_PRINT, KW_INPUT, KW_LET, KW_IF, KW_THEN, KW_ELSE,
    KW_FOR, KW_TO, KW_NEXT, KW_GOTO, KW_GOSUB, KW_RETURN,
    KW_REM, KW_DATA, KW_READ, KW_RESTORE, KW_END, KW_STOP,
    KW_LIST, KW_NEW, KW_RUN, KW_CLEAR, KW_AND, KW_OR, KW_NOT,
    KW_DIM, KW_DEF, KW_FN, KW_ON, KW_STEP
};

enum OperatorType {
    OP_PLUS, OP_MINUS, OP_MULTIPLY, OP_DIVIDE, OP_POWER,
    OP_EQUAL, OP_NOT_EQUAL, OP_LESS, OP_LESS_EQUAL,
    OP_GREATER, OP_GREATER_EQUAL, OP_ASSIGN
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    KeywordType keyword;
    OperatorType operator_type;
    
    Token(TokenType t = TOKEN_EOF, const std::string& v = "", int l = 0, int c = 0) 
        : type(t), value(v), line(l), column(c), keyword(KW_PRINT), operator_type(OP_PLUS) {}
};

class Lexer {
private:
    std::string input;
    size_t position;
    int line;
    int column;
    static std::map<std::string, KeywordType> keywords;
    
    char currentChar();
    char peekChar();
    void advance();
    void skipWhitespace();
    Token readNumber();
    Token readString();
    Token readIdentifier();
    Token readOperator();
    
public:
    Lexer();
    void setInput(const std::string& text);
    Token nextToken();
    std::vector<Token> tokenize(const std::string& text);
    static void initializeKeywords();
};

#endif
