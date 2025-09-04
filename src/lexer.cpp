#include "lexer.h"
#include <cctype>
#include <stdexcept>

std::map<std::string, KeywordType> Lexer::keywords;

Lexer::Lexer() : position(0), line(1), column(1) {
    if (keywords.empty()) {
        initializeKeywords();
    }
}

void Lexer::initializeKeywords() {
    keywords["PRINT"] = KW_PRINT;
    keywords["INPUT"] = KW_INPUT;
    keywords["LET"] = KW_LET;
    keywords["IF"] = KW_IF;
    keywords["THEN"] = KW_THEN;
    keywords["ELSE"] = KW_ELSE;
    keywords["FOR"] = KW_FOR;
    keywords["TO"] = KW_TO;
    keywords["NEXT"] = KW_NEXT;
    keywords["GOTO"] = KW_GOTO;
    keywords["GOSUB"] = KW_GOSUB;
    keywords["RETURN"] = KW_RETURN;
    keywords["REM"] = KW_REM;
    keywords["DATA"] = KW_DATA;
    keywords["READ"] = KW_READ;
    keywords["RESTORE"] = KW_RESTORE;
    keywords["END"] = KW_END;
    keywords["STOP"] = KW_STOP;
    keywords["LIST"] = KW_LIST;
    keywords["NEW"] = KW_NEW;
    keywords["RUN"] = KW_RUN;
    keywords["CLEAR"] = KW_CLEAR;
    keywords["AND"] = KW_AND;
    keywords["OR"] = KW_OR;
    keywords["NOT"] = KW_NOT;
    keywords["DIM"] = KW_DIM;
    keywords["DEF"] = KW_DEF;
    keywords["FN"] = KW_FN;
    keywords["ON"] = KW_ON;
    keywords["STEP"] = KW_STEP;
}

void Lexer::setInput(const std::string& text) {
    input = text;
    position = 0;
    line = 1;
    column = 1;
}

char Lexer::currentChar() {
    if (position >= input.length()) {
        return '\0';
    }
    return input[position];
}

char Lexer::peekChar() {
    if (position + 1 >= input.length()) {
        return '\0';
    }
    return input[position + 1];
}

void Lexer::advance() {
    if (position < input.length() && input[position] == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    position++;
}

void Lexer::skipWhitespace() {
    while (currentChar() == ' ' || currentChar() == '\t') {
        advance();
    }
}

Token Lexer::readNumber() {
    std::string number;
    bool hasDot = false;
    
    while (std::isdigit(currentChar()) || (currentChar() == '.' && !hasDot)) {
        if (currentChar() == '.') {
            hasDot = true;
        }
        number += currentChar();
        advance();
    }
    
    return Token(TOKEN_NUMBER, number, line, column);
}

Token Lexer::readString() {
    std::string str;
    advance(); // Skip opening quote
    
    while (currentChar() != '"' && currentChar() != '\0') {
        str += currentChar();
        advance();
    }
    
    if (currentChar() == '"') {
        advance(); // Skip closing quote
    } else {
        throw std::runtime_error("SYNTAX ERROR");
    }
    
    return Token(TOKEN_STRING, str, line, column);
}

Token Lexer::readIdentifier() {
    std::string identifier;
    
    while (std::isalnum(currentChar()) || currentChar() == '$') {
        identifier += std::toupper(currentChar());
        advance();
    }
    
    auto it = keywords.find(identifier);
    if (it != keywords.end()) {
        Token token(TOKEN_KEYWORD, identifier, line, column);
        token.keyword = it->second;
        return token;
    }
    
    return Token(TOKEN_VARIABLE, identifier, line, column);
}

Token Lexer::readOperator() {
    char ch = currentChar();
    std::string op(1, ch);
    Token token(TOKEN_OPERATOR, op, line, column);
    
    advance();
    
    switch (ch) {
        case '+': token.operator_type = OP_PLUS; break;
        case '-': token.operator_type = OP_MINUS; break;
        case '*': token.operator_type = OP_MULTIPLY; break;
        case '/': token.operator_type = OP_DIVIDE; break;
        case '^': token.operator_type = OP_POWER; break;
        case '=': token.operator_type = OP_EQUAL; break;
        case '<':
            if (currentChar() == '=') {
                token.value += '=';
                token.operator_type = OP_LESS_EQUAL;
                advance();
            } else if (currentChar() == '>') {
                token.value += '>';
                token.operator_type = OP_NOT_EQUAL;
                advance();
            } else {
                token.operator_type = OP_LESS;
            }
            break;
        case '>':
            if (currentChar() == '=') {
                token.value += '=';
                token.operator_type = OP_GREATER_EQUAL;
                advance();
            } else {
                token.operator_type = OP_GREATER;
            }
            break;
    }
    
    return token;
}

Token Lexer::nextToken() {
    skipWhitespace();
    
    char ch = currentChar();
    
    if (ch == '\0') {
        return Token(TOKEN_EOF, "", line, column);
    }
    
    if (ch == '\n' || ch == '\r') {
        advance();
        return Token(TOKEN_NEWLINE, "\n", line, column);
    }
    
    if (std::isdigit(ch) || (ch == '.' && std::isdigit(peekChar()))) {
        return readNumber();
    }
    
    if (ch == '"') {
        return readString();
    }
    
    if (std::isalpha(ch)) {
        return readIdentifier();
    }
    
    if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '^' ||
        ch == '=' || ch == '<' || ch == '>') {
        return readOperator();
    }
    
    if (ch == '(' || ch == ')' || ch == ',' || ch == ';' || ch == ':' || ch == '&' || ch == '\'' || ch == '.' || 
        ch == '[' || ch == ']' || ch == '{' || ch == '}' || ch == '!' || ch == '?' || ch == '#' || ch == '@' || 
        ch == '%' || ch == '$' || ch == '~' || ch == '`' || ch == '|' || ch == '\\') {
        std::string delim(1, ch);
        advance();
        return Token(TOKEN_DELIMITER, delim, line, column);
    }
    
    // Unknown character
    std::string unknown(1, ch);
    advance();
    throw std::runtime_error("SYNTAX ERROR");
}

std::vector<Token> Lexer::tokenize(const std::string& text) {
    setInput(text);
    std::vector<Token> tokens;
    
    Token token;
    do {
        token = nextToken();
        tokens.push_back(token);
    } while (token.type != TOKEN_EOF);
    
    return tokens;
}
