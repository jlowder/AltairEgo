#include "parser.h"
#include "functions.h"
#include <stdexcept>
#include <iostream>

Parser::Parser() : current(0) {}

Token Parser::getCurrentToken() {
    if (current >= tokens.size()) {
        return Token(TOKEN_EOF);
    }
    return tokens[current];
}

Token Parser::peekToken() {
    if (current + 1 >= tokens.size()) {
        return Token(TOKEN_EOF);
    }
    return tokens[current + 1];
}

void Parser::advance() {
    if (current < tokens.size()) {
        current++;
    }
}

bool Parser::match(TokenType type) {
    return getCurrentToken().type == type;
}

bool Parser::matchKeyword(KeywordType keyword) {
    return match(TOKEN_KEYWORD) && getCurrentToken().keyword == keyword;
}

bool Parser::matchOperator(OperatorType op) {
    return match(TOKEN_OPERATOR) && getCurrentToken().operator_type == op;
}

std::shared_ptr<ASTNode> Parser::parse(const std::vector<Token>& tokenList) {
    tokens = tokenList;
    current = 0;
    return parseProgram();
}

std::shared_ptr<ASTNode> Parser::parseProgram() {
    auto program = std::make_shared<ASTNode>(NODE_PROGRAM);
    
    while (!match(TOKEN_EOF)) {
        if (match(TOKEN_NEWLINE)) {
            advance();
            continue;
        }
        
        auto line = parseLine();
        if (line) {
            program->children.push_back(line);
        }
    }
    
    return program;
}

std::shared_ptr<ASTNode> Parser::parseLine() {
    auto line = std::make_shared<ASTNode>(NODE_LINE);
    
    // Check for line number
    if (match(TOKEN_NUMBER)) {
        line->line_number = std::stoi(getCurrentToken().value);
        advance();
    }
    
    // Parse statements
    while (!match(TOKEN_EOF) && !match(TOKEN_NEWLINE)) {
        auto stmt = parseStatement();
        if (stmt) {
            line->children.push_back(stmt);
        }
        
        if (match(TOKEN_DELIMITER) && getCurrentToken().value == ":") {
            advance(); // Skip colon
        } else {
            break;
        }
    }
    
    if (match(TOKEN_NEWLINE)) {
        advance();
    }
    
    return line;
}

std::shared_ptr<ASTNode> Parser::parseStatement() {
    if (matchKeyword(KW_PRINT)) {
        return parsePrintStatement();
    } else if (matchKeyword(KW_INPUT)) {
        return parseInputStatement();
    } else if (matchKeyword(KW_LET)) {
        return parseLetStatement();
    } else if (matchKeyword(KW_IF)) {
        return parseIfStatement();
    } else if (matchKeyword(KW_FOR)) {
        return parseForStatement();
    } else if (matchKeyword(KW_GOTO)) {
        return parseGotoStatement();
    } else if (matchKeyword(KW_GOSUB)) {
        return parseGosubStatement();
    } else if (matchKeyword(KW_RETURN)) {
        return parseReturnStatement();
    } else if (matchKeyword(KW_REM)) {
        return parseRemStatement();
    } else if (matchKeyword(KW_DATA)) {
        return parseDataStatement();
    } else if (matchKeyword(KW_READ)) {
        return parseReadStatement();
    } else if (matchKeyword(KW_RESTORE)) {
        return parseRestoreStatement();
    } else if (matchKeyword(KW_END)) {
        return parseEndStatement();
    } else if (matchKeyword(KW_STOP)) {
        return parseStopStatement();
    } else if (matchKeyword(KW_NEXT)) {
        return parseNextStatement();
    } else if (matchKeyword(KW_ON)) {
        return parseOnStatement();
    } else if (matchKeyword(KW_LIST)) {
        return parseListStatement();
    } else if (matchKeyword(KW_NEW)) {
        return parseNewStatement();
    } else if (matchKeyword(KW_RUN)) {
        return parseRunStatement();
    } else if (matchKeyword(KW_CLEAR)) {
        return parseClearStatement();
    } else if (matchKeyword(KW_DIM)) {
        return parseDimStatement();
    } else if (matchKeyword(KW_DEF)) {
        return parseDefStatement();
    } else if (match(TOKEN_VARIABLE)) {
        // Implicit LET statement
        return parseLetStatement();
    }
    
    throw std::runtime_error("SYNTAX ERROR");
}

std::shared_ptr<ASTNode> Parser::parsePrintStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_PRINT;
    advance(); // Skip PRINT
    
    while (!match(TOKEN_EOF) && !match(TOKEN_NEWLINE) && !(match(TOKEN_DELIMITER) && getCurrentToken().value == ":")) {
        
        if (match(TOKEN_DELIMITER) && getCurrentToken().value == ",") {
            advance();
            auto comma = std::make_shared<ASTNode>(NODE_STRING, ",");
            stmt->children.push_back(comma);
        } else if (match(TOKEN_DELIMITER) && getCurrentToken().value == ";") {
            advance();
            auto semicolon = std::make_shared<ASTNode>(NODE_STRING, ";");
            stmt->children.push_back(semicolon);
        } else {
            auto expr = parseExpression();
            stmt->children.push_back(expr);
        }
    }
    
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseInputStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_INPUT;
    advance(); // Skip INPUT
    
    // Optional prompt string
    if (match(TOKEN_STRING)) {
        auto prompt = std::make_shared<ASTNode>(NODE_STRING, getCurrentToken().value);
        stmt->children.push_back(prompt);
        advance();
        
        // Check for semicolon or comma and mark it
        if (match(TOKEN_DELIMITER)) {
            if (getCurrentToken().value == ";") {
                prompt->value += ";"; // Mark that semicolon was present
                advance();
            } else if (getCurrentToken().value == ",") {
                prompt->value += ","; // Mark that comma was present
                advance();
            }
        }
    }
    
    // Variable list
    auto varList = parseVariableList();
    stmt->children.push_back(varList);
    
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseLetStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_LET;
    
    if (matchKeyword(KW_LET)) {
        advance(); // Skip LET
    }
    
    // Variable assignment (including array access)
    if (match(TOKEN_VARIABLE)) {
        auto var = std::make_shared<ASTNode>(NODE_VARIABLE, getCurrentToken().value);
        advance();
        
        // Check for array access: A(5) = 10 or A(1,2) = 10
        if (match(TOKEN_DELIMITER) && getCurrentToken().value == "(") {
            advance(); // Skip (
            
            auto arrayAccess = std::make_shared<ASTNode>(NODE_ARRAY_ACCESS, var->value);
            arrayAccess->children.push_back(var);
            
            do {
                auto indexExpr = parseExpression();
                arrayAccess->children.push_back(indexExpr);
                
                if (match(TOKEN_DELIMITER) && getCurrentToken().value == ",") {
                    advance(); // Skip comma between indices
                } else {
                    break;
                }
            } while (true);
            
            if (match(TOKEN_DELIMITER) && getCurrentToken().value == ")") {
                advance(); // Skip )
            } else {
                throw std::runtime_error("SYNTAX ERROR");
            }
            
            if (matchOperator(OP_EQUAL)) {
                advance(); // Skip =
                auto expr = parseExpression();
                
                auto assignment = std::make_shared<ASTNode>(NODE_BINARY_OP);
                assignment->operator_type = OP_ASSIGN;
                assignment->children.push_back(arrayAccess);
                assignment->children.push_back(expr);
                
                stmt->children.push_back(assignment);
            } else {
                throw std::runtime_error("SYNTAX ERROR");
            }
        } else if (matchOperator(OP_EQUAL)) {
            // Regular variable assignment: A = 10
            advance(); // Skip =
            auto expr = parseExpression();
            
            auto assignment = std::make_shared<ASTNode>(NODE_BINARY_OP);
            assignment->operator_type = OP_ASSIGN;
            assignment->children.push_back(var);
            assignment->children.push_back(expr);
            
            stmt->children.push_back(assignment);
        } else {
            throw std::runtime_error("SYNTAX ERROR");
        }
    } else {
        throw std::runtime_error("SYNTAX ERROR");
    }
    
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseIfStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_IF;
    advance(); // Skip IF
    
    auto condition = parseExpression();
    stmt->children.push_back(condition);
    
    if (matchKeyword(KW_THEN)) {
        advance(); // Skip THEN
        
        if (match(TOKEN_NUMBER)) {
            // GOTO line number
            auto gotoStmt = std::make_shared<ASTNode>(NODE_STATEMENT);
            gotoStmt->keyword = KW_GOTO;
            auto lineNum = std::make_shared<ASTNode>(NODE_NUMBER, getCurrentToken().value);
            gotoStmt->children.push_back(lineNum);
            stmt->children.push_back(gotoStmt);
            advance();
        } else {
            // Parse all statements in the consequent (separated by colons)
            while (!match(TOKEN_EOF) && !match(TOKEN_NEWLINE)) {
                auto thenStmt = parseStatement();
                stmt->children.push_back(thenStmt);
                
                if (match(TOKEN_DELIMITER) && getCurrentToken().value == ":") {
                    advance(); // Skip colon and continue parsing more statements
                } else {
                    break; // No more statements in the consequent
                }
            }
        }
    } else {
        throw std::runtime_error("SYNTAX ERROR");
    }
    
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseForStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_FOR;
    advance(); // Skip FOR
    
    // Variable
    if (match(TOKEN_VARIABLE)) {
        auto var = std::make_shared<ASTNode>(NODE_VARIABLE, getCurrentToken().value);
        stmt->children.push_back(var);
        advance();
        
        if (matchOperator(OP_EQUAL)) {
            advance(); // Skip =
            auto startExpr = parseExpression();
            stmt->children.push_back(startExpr);
            
            if (matchKeyword(KW_TO)) {
                advance(); // Skip TO
                auto endExpr = parseExpression();
                stmt->children.push_back(endExpr);
                
                // Optional STEP
                if (matchKeyword(KW_STEP)) {
                    advance();
                    auto stepExpr = parseExpression();
                    stmt->children.push_back(stepExpr);
                }
            } else {
                throw std::runtime_error("SYNTAX ERROR");
            }
        } else {
            throw std::runtime_error("SYNTAX ERROR");
        }
    } else {
        throw std::runtime_error("SYNTAX ERROR");
    }
    
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseGotoStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_GOTO;
    advance(); // Skip GOTO
    
    if (match(TOKEN_NUMBER)) {
        auto lineNum = std::make_shared<ASTNode>(NODE_NUMBER, getCurrentToken().value);
        stmt->children.push_back(lineNum);
        advance();
    } else {
        throw std::runtime_error("SYNTAX ERROR");
    }
    
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseGosubStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_GOSUB;
    advance(); // Skip GOSUB
    
    if (match(TOKEN_NUMBER)) {
        auto lineNum = std::make_shared<ASTNode>(NODE_NUMBER, getCurrentToken().value);
        stmt->children.push_back(lineNum);
        advance();
    } else {
        throw std::runtime_error("SYNTAX ERROR");
    }
    
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseReturnStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_RETURN;
    advance(); // Skip RETURN
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseRemStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_REM;
    advance(); // Skip REM
    
    // Everything until end of line is a comment - consume as raw text
    std::string comment;
    while (!match(TOKEN_EOF) && !match(TOKEN_NEWLINE)) {
        if (!comment.empty()) {
            comment += " ";
        }
        comment += getCurrentToken().value;
        advance();
    }
    
    auto commentNode = std::make_shared<ASTNode>(NODE_STRING, comment);
    stmt->children.push_back(commentNode);
    
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseDataStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_DATA;
    advance(); // Skip DATA
    
    while (!match(TOKEN_EOF) && !match(TOKEN_NEWLINE) && 
           !(match(TOKEN_DELIMITER) && getCurrentToken().value == ":")) {
        
        if (match(TOKEN_NUMBER) || match(TOKEN_STRING) || match(TOKEN_VARIABLE)) {
            auto data = std::make_shared<ASTNode>(
                match(TOKEN_NUMBER) ? NODE_NUMBER : NODE_STRING,
                getCurrentToken().value);
            stmt->children.push_back(data);
            advance();
        }
        
        if (match(TOKEN_DELIMITER) && getCurrentToken().value == ",") {
            advance();
        } else {
            break;
        }
    }
    
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseReadStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_READ;
    advance(); // Skip READ
    
    auto varList = parseVariableList();
    stmt->children.push_back(varList);
    
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseRestoreStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_RESTORE;
    advance(); // Skip RESTORE
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseEndStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_END;
    advance(); // Skip END
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseStopStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_STOP;
    advance(); // Skip STOP
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseNextStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_NEXT;
    advance(); // Skip NEXT
    
    // Optional variable
    if (match(TOKEN_VARIABLE)) {
        auto var = std::make_shared<ASTNode>(NODE_VARIABLE, getCurrentToken().value);
        stmt->children.push_back(var);
        advance();
    }
    
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseOnStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_ON;
    advance(); // Skip ON
    
    auto expr = parseExpression();
    stmt->children.push_back(expr);
    
    if (matchKeyword(KW_GOTO) || matchKeyword(KW_GOSUB)) {
        auto action = std::make_shared<ASTNode>(NODE_STATEMENT);
        action->keyword = getCurrentToken().keyword;
        advance();
        
        // Line number list
        while (!match(TOKEN_EOF) && !match(TOKEN_NEWLINE)) {
            if (match(TOKEN_NUMBER)) {
                auto lineNum = std::make_shared<ASTNode>(NODE_NUMBER, getCurrentToken().value);
                action->children.push_back(lineNum);
                advance();
            }
            
            if (match(TOKEN_DELIMITER) && getCurrentToken().value == ",") {
                advance();
            } else {
                break;
            }
        }
        
        stmt->children.push_back(action);
    }
    
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseVariableList() {
    auto varList = std::make_shared<ASTNode>(NODE_EXPRESSION);
    
    while (match(TOKEN_VARIABLE)) {
        auto var = std::make_shared<ASTNode>(NODE_VARIABLE, getCurrentToken().value);
        std::string varName = getCurrentToken().value;
        advance();
        
        // Check for array access like M(N)
        if (match(TOKEN_DELIMITER) && getCurrentToken().value == "(") {
            advance(); // Skip (
            
            auto arrayAccess = std::make_shared<ASTNode>(NODE_ARRAY_ACCESS, varName);
            arrayAccess->children.push_back(var);
            
            do {
                auto indexExpr = parseExpression();
                arrayAccess->children.push_back(indexExpr);
                
                if (match(TOKEN_DELIMITER) && getCurrentToken().value == ",") {
                    advance();
                } else {
                    break;
                }
            } while (true);
            
            if (match(TOKEN_DELIMITER) && getCurrentToken().value == ")") {
                advance(); // Skip )
            } else {
                throw std::runtime_error("SYNTAX ERROR");
            }
            
            varList->children.push_back(arrayAccess);
        } else {
            varList->children.push_back(var);
        }
        
        if (match(TOKEN_DELIMITER) && getCurrentToken().value == ",") {
            advance();
        } else {
            break;
        }
    }
    
    return varList;
}

std::shared_ptr<ASTNode> Parser::parseExpressionList() {
    auto exprList = std::make_shared<ASTNode>(NODE_EXPRESSION);
    
    while (!match(TOKEN_EOF) && !match(TOKEN_NEWLINE)) {
        auto expr = parseExpression();
        exprList->children.push_back(expr);
        
        if (match(TOKEN_DELIMITER) && getCurrentToken().value == ",") {
            advance();
        } else {
            break;
        }
    }
    
    return exprList;
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    return parseLogicalOr();
}

std::shared_ptr<ASTNode> Parser::parseLogicalOr() {
    auto left = parseLogicalAnd();
    
    while (matchKeyword(KW_OR)) {
        auto op = std::make_shared<ASTNode>(NODE_BINARY_OP);
        op->value = "OR";
        advance();
        
        auto right = parseLogicalAnd();
        op->children.push_back(left);
        op->children.push_back(right);
        left = op;
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseLogicalAnd() {
    auto left = parseRelational();
    
    while (matchKeyword(KW_AND)) {
        auto op = std::make_shared<ASTNode>(NODE_BINARY_OP);
        op->value = "AND";
        advance();
        
        auto right = parseRelational();
        op->children.push_back(left);
        op->children.push_back(right);
        left = op;
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseRelational() {
    auto left = parseArithmetic();
    
    while (match(TOKEN_OPERATOR) && 
           (getCurrentToken().operator_type == OP_EQUAL ||
            getCurrentToken().operator_type == OP_NOT_EQUAL ||
            getCurrentToken().operator_type == OP_LESS ||
            getCurrentToken().operator_type == OP_LESS_EQUAL ||
            getCurrentToken().operator_type == OP_GREATER ||
            getCurrentToken().operator_type == OP_GREATER_EQUAL)) {
        
        auto op = std::make_shared<ASTNode>(NODE_BINARY_OP);
        op->operator_type = getCurrentToken().operator_type;
        op->value = getCurrentToken().value;
        advance();
        
        auto right = parseArithmetic();
        op->children.push_back(left);
        op->children.push_back(right);
        left = op;
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseArithmetic() {
    auto left = parseTerm();
    
    while (match(TOKEN_OPERATOR) && 
           (getCurrentToken().operator_type == OP_PLUS ||
            getCurrentToken().operator_type == OP_MINUS)) {
        
        auto op = std::make_shared<ASTNode>(NODE_BINARY_OP);
        op->operator_type = getCurrentToken().operator_type;
        op->value = getCurrentToken().value;
        advance();
        
        auto right = parseTerm();
        op->children.push_back(left);
        op->children.push_back(right);
        left = op;
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseTerm() {
    auto left = parseFactor();
    
    while (match(TOKEN_OPERATOR) && 
           (getCurrentToken().operator_type == OP_MULTIPLY ||
            getCurrentToken().operator_type == OP_DIVIDE)) {
        
        auto op = std::make_shared<ASTNode>(NODE_BINARY_OP);
        op->operator_type = getCurrentToken().operator_type;
        op->value = getCurrentToken().value;
        advance();
        
        auto right = parseFactor();
        op->children.push_back(left);
        op->children.push_back(right);
        left = op;
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseFactor() {
    auto left = parsePrimary();
    
    while (match(TOKEN_OPERATOR) && getCurrentToken().operator_type == OP_POWER) {
        auto op = std::make_shared<ASTNode>(NODE_BINARY_OP);
        op->operator_type = OP_POWER;
        op->value = "^";
        advance();
        
        auto right = parsePrimary();
        op->children.push_back(left);
        op->children.push_back(right);
        left = op;
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parsePrimary() {
    if (match(TOKEN_NUMBER)) {
        auto num = std::make_shared<ASTNode>(NODE_NUMBER, getCurrentToken().value);
        advance();
        return num;
    }
    
    if (match(TOKEN_STRING)) {
        auto str = std::make_shared<ASTNode>(NODE_STRING, getCurrentToken().value);
        advance();
        return str;
    }
    
    if (match(TOKEN_VARIABLE)) {
        auto var = std::make_shared<ASTNode>(NODE_VARIABLE, getCurrentToken().value);
        advance();
        
        // Check for function call or array access
        if (match(TOKEN_DELIMITER) && getCurrentToken().value == "(") {
            advance(); // Skip (
            
            // Check if this is a math function
            if (MathFunctions::isMathFunction(var->value)) {
                auto func = std::make_shared<ASTNode>(NODE_FUNCTION_CALL, var->value);
                
                if (!(match(TOKEN_DELIMITER) && getCurrentToken().value == ")")) {
                    do {
                        auto arg = parseExpression();
                        func->children.push_back(arg);
                        
                        if (match(TOKEN_DELIMITER) && getCurrentToken().value == ",") {
                            advance();
                        } else {
                            break;
                        }
                    } while (true);
                }
                
                if (match(TOKEN_DELIMITER) && getCurrentToken().value == ")") {
                    advance(); // Skip )
                } else {
                    throw std::runtime_error("SYNTAX ERROR");
                }
                
                return func;
            } else if (MathFunctions::isStringFunction(var->value)) {
                // Handle string functions like CHR$, LEFT$, RIGHT$, etc.
                auto func = std::make_shared<ASTNode>(NODE_STRING_FUNCTION_CALL, var->value);
                
                if (!(match(TOKEN_DELIMITER) && getCurrentToken().value == ")")) {
                    do {
                        auto arg = parseExpression();
                        func->children.push_back(arg);
                        
                        if (match(TOKEN_DELIMITER) && getCurrentToken().value == ",") {
                            advance();
                        } else {
                            break;
                        }
                    } while (true);
                }
                
                if (match(TOKEN_DELIMITER) && getCurrentToken().value == ")") {
                    advance(); // Skip )
                } else {
                    throw std::runtime_error("SYNTAX ERROR");
                }
                
                return func;
            } else {
                // This is array access - could be A(1) or A(1,2) etc.
                auto arrayAccess = std::make_shared<ASTNode>(NODE_ARRAY_ACCESS, var->value);
                arrayAccess->children.push_back(var);
                
                do {
                    auto indexExpr = parseExpression();
                    arrayAccess->children.push_back(indexExpr);
                    
                    if (match(TOKEN_DELIMITER) && getCurrentToken().value == ",") {
                        advance(); // Skip comma between indices
                    } else {
                        break;
                    }
                } while (true);
                
                if (match(TOKEN_DELIMITER) && getCurrentToken().value == ")") {
                    advance(); // Skip )
                } else {
                    throw std::runtime_error("SYNTAX ERROR");
                }
                
                return arrayAccess;
            }
        }
        
        return var;
    }
    
    if (match(TOKEN_DELIMITER) && getCurrentToken().value == "(") {
        advance(); // Skip (
        auto expr = parseExpression();
        
        if (match(TOKEN_DELIMITER) && getCurrentToken().value == ")") {
            advance(); // Skip )
            return expr;
        } else {
            throw std::runtime_error("SYNTAX ERROR");
        }
    }
    
    if (match(TOKEN_OPERATOR) && getCurrentToken().operator_type == OP_MINUS) {
        auto unary = std::make_shared<ASTNode>(NODE_UNARY_OP);
        unary->operator_type = OP_MINUS;
        unary->value = "-";
        advance();
        
        auto operand = parsePrimary();
        unary->children.push_back(operand);
        return unary;
    }
    
    if (matchKeyword(KW_NOT)) {
        auto unary = std::make_shared<ASTNode>(NODE_UNARY_OP);
        unary->value = "NOT";
        advance();
        
        auto operand = parsePrimary();
        unary->children.push_back(operand);
        return unary;
    }
    
    throw std::runtime_error("SYNTAX ERROR");
}

std::shared_ptr<ASTNode> Parser::parseListStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_LIST;
    advance(); // Skip LIST
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseNewStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_NEW;
    advance(); // Skip NEW
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseRunStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_RUN;
    advance(); // Skip RUN
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseClearStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_CLEAR;
    advance(); // Skip CLEAR
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseDimStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_DIM;
    advance(); // Skip DIM
    
    // Parse array declarations: DIM A(10), B(20), C(5)
    do {
        if (!match(TOKEN_VARIABLE)) {
            throw std::runtime_error("SYNTAX ERROR");
        }
        
        auto arrayName = std::make_shared<ASTNode>(NODE_VARIABLE, getCurrentToken().value);
        advance();
        
        if (!match(TOKEN_DELIMITER) || getCurrentToken().value != "(") {
            throw std::runtime_error("SYNTAX ERROR");
        }
        advance(); // Skip (
        
        // Parse dimensions: could be A(10) or A(10,20) etc.
        auto dimDecl = std::make_shared<ASTNode>(NODE_DIM_DECLARATION);
        dimDecl->children.push_back(arrayName);
        
        do {
            auto sizeExpr = parseExpression();
            dimDecl->children.push_back(sizeExpr);
            
            if (match(TOKEN_DELIMITER) && getCurrentToken().value == ",") {
                advance(); // Skip comma between dimensions
            } else {
                break;
            }
        } while (true);
        
        if (!match(TOKEN_DELIMITER) || getCurrentToken().value != ")") {
            throw std::runtime_error("SYNTAX ERROR");
        }
        advance(); // Skip )
        stmt->children.push_back(dimDecl);
        
        if (match(TOKEN_DELIMITER) && getCurrentToken().value == ",") {
            advance(); // Skip comma for multiple declarations
        } else {
            break;
        }
    } while (true);
    
    return stmt;
}

std::shared_ptr<ASTNode> Parser::parseDefStatement() {
    auto stmt = std::make_shared<ASTNode>(NODE_STATEMENT);
    stmt->keyword = KW_DEF;
    advance(); // Skip DEF
    
    // Parse function name (e.g., FNA)
    if (!match(TOKEN_VARIABLE)) {
        throw std::runtime_error("SYNTAX ERROR");
    }
    auto funcName = std::make_shared<ASTNode>(NODE_VARIABLE, getCurrentToken().value);
    stmt->children.push_back(funcName);
    advance();
    
    // Parse parameter list: (Z)
    if (!match(TOKEN_DELIMITER) || getCurrentToken().value != "(") {
        throw std::runtime_error("SYNTAX ERROR");
    }
    advance(); // Skip (
    
    if (!match(TOKEN_VARIABLE)) {
        throw std::runtime_error("SYNTAX ERROR");
    }
    auto param = std::make_shared<ASTNode>(NODE_VARIABLE, getCurrentToken().value);
    stmt->children.push_back(param);
    advance();
    
    if (!match(TOKEN_DELIMITER) || getCurrentToken().value != ")") {
        throw std::runtime_error("SYNTAX ERROR");
    }
    advance(); // Skip )
    
    // Parse = sign
    if (!matchOperator(OP_EQUAL)) {
        throw std::runtime_error("SYNTAX ERROR");
    }
    advance();
    
    // Parse function body expression
    auto body = parseExpression();
    stmt->children.push_back(body);
    
    return stmt;
}
