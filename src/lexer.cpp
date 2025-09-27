#include "lexer.h"
#include <cctype>
#include <stdexcept>
#include <sstream>
#include <string>
#include <algorithm>

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

std::string Lexer::getLineText(int lineNumber) {
    std::stringstream ss(input);
    std::string line;
    int currentLine = 1;
    while (std::getline(ss, line)) {
        if (currentLine == lineNumber) {
            return line;
        }
        currentLine++;
    }
    return "";
}

void Lexer::syntaxError() {
    std::string lineText = getLineText(line);
    // Trim leading/trailing whitespace from lineText for cleaner output
    lineText.erase(0, lineText.find_first_not_of(" \t\n\r"));
    lineText.erase(lineText.find_last_not_of(" \t\n\r") + 1);
    std::string errorMessage = "SYNTAX ERROR in line " + std::to_string(line) + ": " + lineText;
    throw std::runtime_error(errorMessage);
}

void Lexer::setInput(const std::string& text) {
    input = text;
    position = 0;
    line = 1;
    column = 1;
    tokenBuffer.clear();
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
        syntaxError();
    }
    
    return Token(TOKEN_STRING, str, line, column);
}

bool Lexer::isKeywordPrefix(const std::string& text, size_t startPos, std::string& keyword) {
    // Check if there's a keyword starting at the given position
    for (const auto& pair : keywords) {
        const std::string& kw = pair.first;
        if (startPos + kw.length() <= text.length()) {
            std::string substr = text.substr(startPos, kw.length());
            std::transform(substr.begin(), substr.end(), substr.begin(), ::toupper);
            if (substr == kw) {
                // Special case: FN should not be treated as a keyword prefix
                // when followed by alphabetic characters, because FND, FNR, etc.
                // are complete function names, not "FN" keyword + variable
                if (kw == "FN") {
                    if (startPos + kw.length() < text.length()) {
                        char nextChar = text[startPos + kw.length()];
                        if (std::isalpha(nextChar)) {
                            continue; // Skip FN when followed by alphabetic character
                        }
                    }
                }
                
                // Check if the character after the keyword is valid for a variable start
                if (startPos + kw.length() < text.length()) {
                    char nextChar = text[startPos + kw.length()];
                    if (std::isalpha(nextChar) || nextChar == '$') {
                        keyword = kw;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool Lexer::isKeywordSuffix(const std::string& text, size_t startPos, std::string& keyword) {
    // Check if there's a keyword ending at the given position
    for (const auto& pair : keywords) {
        const std::string& kw = pair.first;
        if (startPos >= kw.length()) {
            std::string substr = text.substr(startPos - kw.length(), kw.length());
            std::transform(substr.begin(), substr.end(), substr.begin(), ::toupper);
            if (substr == kw) {
                // Check if the character after the keyword is valid for a number start
                if (startPos < text.length()) {
                    char nextChar = text[startPos];
                    if (std::isdigit(nextChar)) {
                        keyword = kw;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool Lexer::canSplitIdentifier(const std::string& identifier, std::string& keyword, std::string& remainder) {
    // Check if identifier contains a keyword followed by a number
    for (const auto& pair : keywords) {
        const std::string& kw = pair.first;
        if (identifier.length() > kw.length()) {
            std::string prefix = identifier.substr(0, kw.length());
            std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::toupper);
            if (prefix == kw) {
                // Check if the remainder is a valid number
                std::string suffix = identifier.substr(kw.length());
                if (!suffix.empty() && std::all_of(suffix.begin(), suffix.end(), ::isdigit)) {
                    keyword = kw;
                    remainder = suffix;
                    return true;
                }
            }
        }
    }
    return false;
}

bool Lexer::canSplitIdentifierWithEmbeddedKeyword(const std::string& identifier, std::string& prefix, std::string& keyword, std::string& suffix) {
    // Check if identifier contains a keyword embedded within it (like T9THENT9 -> T9 + THEN + T9)
    // or multiple keywords in sequence (like ONIGOTO2300 -> ON + I + GOTO + 2300)
    
    // First, try to find a keyword at the beginning
    for (const auto& pair : keywords) {
        const std::string& kw = pair.first;
        if (identifier.length() >= kw.length()) {
            std::string substr = identifier.substr(0, kw.length());
            std::transform(substr.begin(), substr.end(), substr.begin(), ::toupper);
            if (substr == kw) {
                // Found keyword at the beginning
                prefix = "";
                keyword = kw;
                suffix = identifier.substr(kw.length());
                return true;
            }
        }
    }
    
    // Then, try to find a keyword embedded within the identifier
    for (const auto& pair : keywords) {
        const std::string& kw = pair.first;
        if (identifier.length() > kw.length()) {
            // Look for the keyword at any position within the identifier
            for (size_t i = 1; i <= identifier.length() - kw.length(); i++) {
                std::string substr = identifier.substr(i, kw.length());
                std::transform(substr.begin(), substr.end(), substr.begin(), ::toupper);
                if (substr == kw) {
                    // Found the keyword at position i
                    prefix = identifier.substr(0, i);
                    keyword = kw;
                    suffix = identifier.substr(i + kw.length());
                    return true;
                }
            }
        }
    }
    return false;
}

void Lexer::splitIdentifierRecursively(const std::string& identifier, std::vector<Token>& tokens) {
    // Check if this identifier can be split with embedded keyword
    std::string prefix, keyword, suffix;
    if (canSplitIdentifierWithEmbeddedKeyword(identifier, prefix, keyword, suffix)) {
        // Recursively split the prefix if it's not empty
        if (!prefix.empty()) {
            splitIdentifierRecursively(prefix, tokens);
        }
        
        // Add the keyword token
        auto it = keywords.find(keyword);
        Token keywordToken(TOKEN_KEYWORD, keyword, line, column);
        keywordToken.keyword = it->second;
        tokens.push_back(keywordToken);
        
        // Recursively split the suffix if it's not empty
        if (!suffix.empty()) {
            splitIdentifierRecursively(suffix, tokens);
        }
        return;
    }
    
    // Check if this identifier can be split (like TO9 -> TO + 9)
    std::string keyword2, remainder;
    if (canSplitIdentifier(identifier, keyword2, remainder)) {
        // Add the keyword token
        auto it = keywords.find(keyword2);
        Token keywordToken(TOKEN_KEYWORD, keyword2, line, column);
        keywordToken.keyword = it->second;
        tokens.push_back(keywordToken);
        
        // Add the number token
        Token numberToken(TOKEN_NUMBER, remainder, line, column);
        tokens.push_back(numberToken);
        return;
    }
    
    // Check if it's a complete keyword
    auto it = keywords.find(identifier);
    if (it != keywords.end()) {
        Token token(TOKEN_KEYWORD, identifier, line, column);
        token.keyword = it->second;
        tokens.push_back(token);
        return;
    }
    
    // It's a variable or number
    if (!identifier.empty() && std::isdigit(identifier[0])) {
        Token token(TOKEN_NUMBER, identifier, line, column);
        tokens.push_back(token);
    } else {
        Token token(TOKEN_VARIABLE, identifier, line, column);
        tokens.push_back(token);
    }
}

Token Lexer::readIdentifier() {
    std::string identifier;
    
    while (std::isalnum(currentChar()) || currentChar() == '$') {
        identifier += std::toupper(currentChar());
        advance();
    }
    
    // Split the identifier recursively
    std::vector<Token> tokens;
    splitIdentifierRecursively(identifier, tokens);
    
    // Buffer all tokens except the first one
    for (int i = tokens.size() - 1; i > 0; i--) {
        tokenBuffer.push_back(tokens[i]);
    }
    
    // Return the first token
    if (!tokens.empty()) {
        return tokens[0];
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
    // Check if we have buffered tokens to return
    if (!tokenBuffer.empty()) {
        Token token = tokenBuffer.back();
        tokenBuffer.pop_back();
        return token;
    }
    
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
        // Check if this could be a keyword followed by a variable
        std::string keyword;
        if (isKeywordPrefix(input, position, keyword)) {
            // Read the keyword
            for (size_t i = 0; i < keyword.length(); i++) {
                advance();
            }
            auto it = keywords.find(keyword);
            Token token(TOKEN_KEYWORD, keyword, line, column);
            token.keyword = it->second;
            return token;
        }
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
    syntaxError();
    return Token(); // Should be unreachable
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
