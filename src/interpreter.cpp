#include "interpreter.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <set>

#ifdef __EMSCRIPTEN__
#include <sstream> // Required for std::stringstream
extern std::stringstream output_buffer;
extern "C" {
    void flush_output_to_js(const char* text);
    void await_input_from_js();
    const char* get_input_buffer();
}
#endif

AltairBasicInterpreter::AltairBasicInterpreter() 
    : dataPointer(0), currentLine(-1), currentStatementIndex(0), running(false), stopExecution(false), returningFromSubroutine(false), debug(false), m_currentColumn(0) {}

void AltairBasicInterpreter::processLine(const std::string& input) {
    if (input == "DEBUG ON") {
        debug = true;
        std::cout << "Debugging enabled." << std::endl;
        return;
    } else if (input == "DEBUG OFF") {
        debug = false;
        std::cout << "Debugging disabled." << std::endl;
        return;
    }

    DEBUG_PRINT("Processing line: " << input);

    try {
        auto tokens = lexer.tokenize(input);
        auto ast = parser.parse(tokens);
        
        if (!ast || ast->children.empty()) {
            return;
        }
        
        auto line = ast->children[0];
        
        if (isDirectMode(line)) {
            // Direct mode - execute immediately
            if (line->children.empty()) {
                return;
            }
            
            auto stmt = line->children[0];
            if (isCommand(stmt)) {
                executeStatement(stmt);
            } else {
                executeLine(line);
            }
            
            if (!running) {
                std::cout << "OK" << std::endl;
            }
        } else {
            // Indirect mode - store in program
            int lineNum = line->line_number;
            if (lineNum < 1 || lineNum > 65529) {
                throw std::runtime_error("ILLEGAL LINE NUMBER");
            }
            
            if (line->children.empty()) {
                // Delete line
                program.erase(lineNum);
            } else {
                // Store line
                program.emplace(lineNum, ProgramLine(lineNum, line));
            }
        }
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        if (running) {
            running = false;
            std::cout << "OK" << std::endl;
        }
    }
}

bool AltairBasicInterpreter::isDirectMode(std::shared_ptr<ASTNode> line) {
    return line->line_number == 0;
}

bool AltairBasicInterpreter::isCommand(std::shared_ptr<ASTNode> stmt) {
    if (stmt->type != NODE_STATEMENT) return false;
    
    return stmt->keyword == KW_LIST || stmt->keyword == KW_NEW ||
           stmt->keyword == KW_RUN || stmt->keyword == KW_CLEAR ||
           stmt->keyword == KW_GOTO || stmt->keyword == KW_GOSUB;
}

void AltairBasicInterpreter::executeStatement(std::shared_ptr<ASTNode> stmt) {
    DEBUG_PRINT("Executing statement: " << stmt->keyword);
    switch (stmt->keyword) {
        case KW_PRINT:
            executePrint(stmt);
            break;
        case KW_INPUT:
            executeInput(stmt);
            break;
        case KW_LET:
            executeLet(stmt);
            break;
        case KW_IF:
            executeIf(stmt);
            break;
        case KW_FOR:
            executeFor(stmt);
            break;
        case KW_NEXT:
            executeNext(stmt);
            break;
        case KW_GOTO:
            executeGoto(stmt);
            break;
        case KW_GOSUB:
            executeGosub(stmt);
            break;
        case KW_RETURN:
            executeReturn(stmt);
            break;
        case KW_DATA:
            executeData(stmt);
            break;
        case KW_READ:
            executeRead(stmt);
            break;
        case KW_RESTORE:
            executeRestore(stmt);
            break;
        case KW_END:
            executeEnd(stmt);
            break;
        case KW_STOP:
            executeStop(stmt);
            break;
        case KW_ON:
            executeOn(stmt);
            break;
        case KW_LIST:
            executeList();
            break;
        case KW_NEW:
            executeNew();
            break;
        case KW_RUN:
            executeRun();
            break;
        case KW_CLEAR:
            executeClear();
            break;
        case KW_DIM:
            executeDim(stmt);
            break;
        case KW_DEF:
            executeDef(stmt);
            break;
        case KW_REM:
            // Comments do nothing
            break;
        default:
            throw std::runtime_error("UNDEFINED STATEMENT");
    }
}

void AltairBasicInterpreter::executePrint(std::shared_ptr<ASTNode> stmt) {
    bool newlineAtEnd = true;

    // Check if the statement contains anything other than TAB calls and separators
    bool hasContent = false;
    for (const auto& child : stmt->children) {
        if (child->type == NODE_FUNCTION_CALL && child->value == "TAB") {
            continue;
        }
        if (child->type == NODE_STRING && (child->value == "," || child->value == ";")) {
            continue;
        }
        hasContent = true;
        break;
    }

    for (size_t i = 0; i < stmt->children.size(); ++i) {
        auto child = stmt->children[i];
        
        if (child->type == NODE_STRING) {
            if (child->value == ",") {
                // Tab to next print zone (every 14 characters)
                int nextZone = ((m_currentColumn / 14) + 1) * 14;
                int spaces = nextZone - m_currentColumn;
                for (int j = 0; j < spaces; j++) {
                    std::cout << " ";
                }
                m_currentColumn = nextZone;
            } else if (child->value == ";") {
                // Semicolon only suppresses newline if it's the last item
                if (i == stmt->children.size() - 1) {
                    newlineAtEnd = false;
                }
                // Otherwise, semicolon just means no spacing between items
            } else {
                std::cout << child->value;
                m_currentColumn += child->value.length();
            }
        } else if (child->type == NODE_FUNCTION_CALL && child->value == "TAB") {
            // Special handling for TAB() function
            if (!child->children.empty()) {
                // TAB is 1-indexed, so subtract 1. TAB(1) goes to column 0.
                int targetColumn = static_cast<int>(evaluateExpression(child->children[0])) - 1;
                if (targetColumn < 0) targetColumn = 0;
                if (targetColumn > 255) targetColumn = 255;
                
                if (targetColumn > m_currentColumn) {
                    int spaces = targetColumn - m_currentColumn;
                    for (int j = 0; j < spaces; j++) {
                        std::cout << " ";
                    }
                    m_currentColumn = targetColumn;
                }
                // If targetColumn <= m_currentColumn, do nothing (can't move backwards)
            }
        } else if (child->type == NODE_STRING_FUNCTION_CALL) {
            // Handle string function calls like CHR$(65)
            std::string output = evaluateStringExpression(child);
            std::cout << output;
            m_currentColumn += output.length();
        } else if (child->type == NODE_VARIABLE && child->value.back() == '$') {
            // Handle string variables like A$, G2$
            std::string output = variables.getStringVariable(child->value);
            std::cout << output;
            m_currentColumn += output.length();
        } else if (child->type == NODE_ARRAY_ACCESS && child->children.size() >= 1 && child->children[0]->value.back() == '$') {
            // Handle string arrays like A$(1)
            std::string output = evaluateStringExpression(child);
            std::cout << output;
            m_currentColumn += output.length();
        } else {
            std::string output = formatNumber(evaluateExpression(child));
            std::cout << output;
            m_currentColumn += output.length();
        }
    }
    
    // If a print statement has no content (only TABs, commas, semicolons),
    // it should not print a newline unless it's an empty PRINT statement.
    if (!hasContent && !stmt->children.empty()) {
        newlineAtEnd = false;
    }

    if (newlineAtEnd) {
        std::cout << std::endl;
        m_currentColumn = 0;
    }
}

void AltairBasicInterpreter::executeInput(std::shared_ptr<ASTNode> stmt) {
    size_t startIndex = 0;
    
    // Check for prompt string
    if (!stmt->children.empty() && stmt->children[0]->type == NODE_STRING) {
        std::string promptValue = stmt->children[0]->value;
        bool hasSemicolon = promptValue.back() == ';';
        bool hasComma = promptValue.back() == ',';
        
        if (hasSemicolon) {
            std::cout << promptValue.substr(0, promptValue.length() - 1) << "? ";
        } else if (hasComma) {
            std::cout << promptValue.substr(0, promptValue.length() - 1) << "?";
        } else {
            std::cout << promptValue << "?";
        }
        startIndex = 1;
    } else {
        std::cout << "? ";
    }
    
    // Get variable list
    if (startIndex >= stmt->children.size()) {
        // INPUT statement with no variables, just consume a line of input
        std::string dummy;
        std::getline(std::cin, dummy);
        return;
    }

    auto varList = stmt->children[startIndex];
    std::vector<std::string> allValues;

    while (true) {
        allValues.clear();
        std::string currentInputLine;

        while(allValues.size() < varList->children.size()) {
#ifdef __EMSCRIPTEN__
            std::string current_output = output_buffer.str();
            if (!current_output.empty()) {
                flush_output_to_js(current_output.c_str());
                output_buffer.str("");
                output_buffer.clear();
            }
            await_input_from_js();
            currentInputLine = get_input_buffer();
#else
            if (!std::getline(std::cin, currentInputLine)) {
                return; // End of input stream
            }
#endif

            std::stringstream ss(currentInputLine);
            std::string value;
            while (std::getline(ss, value, ',')) {
                size_t start = value.find_first_not_of(" \t");
                size_t end = value.find_last_not_of(" \t");
                if (start != std::string::npos) {
                    allValues.push_back(value.substr(start, end - start + 1));
                } else {
                    allValues.push_back("");
                }
            }
            if (allValues.size() < varList->children.size()) {
                std::cout << "?? ";
            }
        }

        bool success = true;
        for (size_t i = 0; i < varList->children.size(); ++i) {
            auto var = varList->children[i];
            if (var->value.back() == '$') {
                // String variable, no validation needed
            } else {
                // Numeric variable, check if it's a valid number
                try {
                    std::stod(allValues[i]);
                } catch (const std::exception&) {
                    success = false;
                    break;
                }
            }
        }

        if (success) {
            // All inputs are valid, now assign them
            for (size_t i = 0; i < varList->children.size(); ++i) {
                auto var = varList->children[i];
                if (var->value.back() == '$') {
                    variables.setStringVariable(var->value, allValues[i]);
                } else {
                    variables.setNumericVariable(var->value, std::stod(allValues[i]));
                }
            }
            break; // Exit the while(true) loop
        } else {
            std::cout << "REDO FROM START" << std::endl;
            std::cout << "? ";
        }
    }
}

void AltairBasicInterpreter::executeLet(std::shared_ptr<ASTNode> stmt) {
    if (stmt->children.empty()) return;
    
    auto assignment = stmt->children[0];
    if (assignment->type == NODE_BINARY_OP && assignment->operator_type == OP_ASSIGN) {
        auto var = assignment->children[0];
        auto expr = assignment->children[1];
        
        // Check if this is a string variable assignment
        if (var->type == NODE_VARIABLE && var->value.back() == '$') {
            // String variable assignment: G2$ = "SHIELD CONTROL"
            std::string stringValue = evaluateStringExpression(expr);
            DEBUG_PRINT("  LET " << var->value << " = "" << stringValue << """);
            variables.setStringVariable(var->value, stringValue);
        } else if (var->type == NODE_ARRAY_ACCESS && var->children.size() >= 2) {
            // Array assignment: check if it's a string array
            auto arrayName = var->children[0];
            if (arrayName->value.back() == '$') {
                // String array assignment: A$(S) = MID$(L$,Q(S),1)
                std::string stringValue = evaluateStringExpression(expr);
                
                if (var->children.size() == 2) {
                    // Single dimension: A$(5) = "hello"
                    auto indexExpr = var->children[1];
                    int index = static_cast<int>(evaluateExpression(indexExpr));
                    DEBUG_PRINT("  LET " << arrayName->value << "(" << index << ") = "" << stringValue << """);
                    variables.setStringArrayElement(arrayName->value, index, stringValue);
                } else {
                    // Multi-dimensional: A$(1,2) = "hello"
                    std::vector<int> indices;
                    for (size_t i = 1; i < var->children.size(); i++) {
                        int index = static_cast<int>(evaluateExpression(var->children[i]));
                        indices.push_back(index);
                    }
                    variables.setStringArrayElement(arrayName->value, indices, stringValue);
                }
            } else {
                // Numeric array assignment
                double value = evaluateExpression(expr);
                
                if (var->children.size() == 2) {
                    // Single dimension: A(5) = 10
                    auto indexExpr = var->children[1];
                    int index = static_cast<int>(evaluateExpression(indexExpr));
                    DEBUG_PRINT("  LET " << arrayName->value << "(" << index << ") = " << value);
                    variables.setArrayElement(arrayName->value, index, value);
                } else {
                    // Multi-dimensional: A(1,2) = 10
                    std::vector<int> indices;
                    for (size_t i = 1; i < var->children.size(); i++) {
                        int index = static_cast<int>(evaluateExpression(var->children[i]));
                        indices.push_back(index);
                    }
                    variables.setArrayElement(arrayName->value, indices, value);
                }
            }
        } else {
            // Numeric variable assignment
            double value = evaluateExpression(expr);
            
            if (var->type == NODE_ARRAY_ACCESS) {
                // This case should be handled above, but keeping for safety
                if (var->children.size() >= 2) {
                    auto arrayName = var->children[0];
                    
                    if (var->children.size() == 2) {
                        // Single dimension: A(5) = 10
                        auto indexExpr = var->children[1];
                        int index = static_cast<int>(evaluateExpression(indexExpr));
                        DEBUG_PRINT("  LET " << arrayName->value << "(" << index << ") = " << value);
                        variables.setArrayElement(arrayName->value, index, value);
                    } else {
                        // Multi-dimensional: A(1,2) = 10
                        std::vector<int> indices;
                        for (size_t i = 1; i < var->children.size(); i++) {
                            int index = static_cast<int>(evaluateExpression(var->children[i]));
                            indices.push_back(index);
                        }
                        variables.setArrayElement(arrayName->value, indices, value);
                    }
                }
            } else {
                // Regular variable assignment: A = 10
                DEBUG_PRINT("  LET " << var->value << " = " << value);
                variables.setNumericVariable(var->value, value);
            }
        }
    }
}

void AltairBasicInterpreter::executeIf(std::shared_ptr<ASTNode> stmt) {
    if (stmt->children.size() < 2) return;
    
    auto condition = stmt->children[0];
    double conditionValue = evaluateExpression(condition);
    
    if (conditionValue != 0.0) { // Non-zero is true in BASIC
        // Execute all statements in the consequent (starting from index 1)
        for (size_t i = 1; i < stmt->children.size(); i++) {
            executeStatement(stmt->children[i]);
        }
    }
}

void AltairBasicInterpreter::executeGoto(std::shared_ptr<ASTNode> stmt) {
    if (stmt->children.empty()) return;
    
    auto lineNumNode = stmt->children[0];
    int lineNumber = static_cast<int>(evaluateExpression(lineNumNode));
    
    DEBUG_PRINT("GOTO " << lineNumber);

    // Clean up FOR loop stack when jumping out of loops
    // Check if GOTO jumps over any NEXT statements, indicating those loops are exited
    cleanupForLoopStackOnGoto(currentLine, lineNumber);
    
    gotoLine(lineNumber);
    
    // If we're not already running a program (i.e., direct mode), start execution
    if (!running) {
        executeProgram();
    }
}

void AltairBasicInterpreter::executeGosub(std::shared_ptr<ASTNode> stmt) {
    if (stmt->children.empty()) return;
    
    auto lineNumNode = stmt->children[0];
    int lineNumber = static_cast<int>(evaluateExpression(lineNumNode));
    
    DEBUG_PRINT("GOSUB from line " << currentLine << " stmt " << currentStatementIndex 
              << " to line " << lineNumber << ", callStack size: " << callStack.size() << ", forLoopStack size: " << forLoopStack.size());
    
    // Push call frame with return position AFTER this GOSUB statement
    callStack.push(CallFrame(currentLine, currentStatementIndex + 1));
    DEBUG_PRINT("  After GOSUB push, callStack size: " << callStack.size() << ", forLoopStack size: " << forLoopStack.size());
    
    // Jump to subroutine line
    currentLine = lineNumber;
    currentStatementIndex = -1; // Will start at 0 when executeLine runs
}

void AltairBasicInterpreter::executeReturn(std::shared_ptr<ASTNode> stmt) {
    if (callStack.empty()) {
        throw std::runtime_error("RETURN WITHOUT GOSUB");
    }
    
    // Pop call frame and restore execution position
    CallFrame frame = callStack.top();
    callStack.pop();
    
    DEBUG_PRINT("RETURN to line " << frame.returnLine 
              << " stmt " << frame.returnStatementIndex << ", callStack size: " << callStack.size() << ", forLoopStack size: " << forLoopStack.size());
    
    // Jump back to the line and statement after the GOSUB.
    currentLine = frame.returnLine;
    // Set currentStatementIndex to the exact position we want to continue from
    // The main loop will call executeLine which will continue from this position
    currentStatementIndex = frame.returnStatementIndex;
}

void AltairBasicInterpreter::executeLine(std::shared_ptr<ASTNode> line) {
    // If currentStatementIndex is -1, start from 0
    if (currentStatementIndex < 0) {
        currentStatementIndex = 0;
    }

    DEBUG_PRINT("Executing line " << currentLine
              << " starting from stmt " << currentStatementIndex << ", callStack size: " << callStack.size() << ", forLoopStack size: " << forLoopStack.size());

    for (; currentStatementIndex < static_cast<int>(line->children.size()); currentStatementIndex++) {
        if (stopExecution) break;

        int originalLine = currentLine;
        int originalStatementIndex = currentStatementIndex;

        DEBUG_PRINT("About to execute stmt " << currentStatementIndex
                  << " on line " << currentLine);

        executeStatement(line->children[currentStatementIndex]);

        // Check if execution jumped to a different line
        if (currentLine != originalLine) {
            // Execution jumped to different line - break out of current line processing
            DEBUG_PRINT("Line changed from " << originalLine
                      << " to " << currentLine);
            break;
        }

        // No special handling needed here anymore. The for loop's increment
        // will handle moving to the next statement on the same line.
        // Jumps (GOTO, GOSUB, RETURN, NEXT) are handled by breaking out
        // of this loop because currentLine changes.
    }
}

void AltairBasicInterpreter::executeFor(std::shared_ptr<ASTNode> stmt) {
    if (stmt->children.size() < 3) return;
    
    auto var = stmt->children[0];
    auto startExpr = stmt->children[1];
    auto endExpr = stmt->children[2];
    
    double startValue = evaluateExpression(startExpr);
    double endValue = evaluateExpression(endExpr);
    double stepValue = 1.0;
    
    if (stmt->children.size() > 3) {
        stepValue = evaluateExpression(stmt->children[3]);
    }
    
    DEBUG_PRINT("FOR " << var->value << " = " << startValue 
              << " TO " << endValue << " STEP " << stepValue);
    
    variables.setNumericVariable(var->value, startValue);
    
    // Check if loop should execute at all (authentic Altair BASIC behavior)
    bool shouldExecute = false;
    if (stepValue > 0) {
        shouldExecute = startValue <= endValue;
    } else {
        shouldExecute = startValue >= endValue;
    }
    
    if (shouldExecute) {
        // Store the FIRST STATEMENT AFTER THE FOR to return to
        // This should be the PRINT statement, not back to FOR
        int returnLine = currentLine;
        int returnStmtIndex = currentStatementIndex + 1; // Next statement after FOR
        
        // Check if there are more statements after FOR on same line
        auto it = program.find(currentLine);
        if (it != program.end() && returnStmtIndex < static_cast<int>(it->second.ast->children.size())) {
            // More statements on this line - return to next statement after FOR
            DEBUG_PRINT("FOR will return to line " << returnLine 
                      << " stmt " << returnStmtIndex << ", forLoopStack size: " << forLoopStack.size());
            ForLoopState loopState(var->value, endValue, stepValue, returnLine, returnStmtIndex);
            forLoopStack.push(loopState);
            DEBUG_PRINT("  After FOR push, forLoopStack size: " << forLoopStack.size());
        } else {
            // No more statements on this line, go to next line
            int nextLine = getNextLineNumber(currentLine);
            DEBUG_PRINT("FOR will return to line " << nextLine << ", forLoopStack size: " << forLoopStack.size());
            ForLoopState loopState(var->value, endValue, stepValue, nextLine);
            forLoopStack.push(loopState);
            DEBUG_PRINT("  After FOR push, forLoopStack size: " << forLoopStack.size());
        }
    } else {
        // Skip the entire loop by jumping to the line after the matching NEXT
        findMatchingNext(currentLine);
    }
}

void AltairBasicInterpreter::executeNext(std::shared_ptr<ASTNode> stmt) {
    if (forLoopStack.empty()) {
        throw std::runtime_error("NEXT WITHOUT FOR");
    }
    
    auto loopState = forLoopStack.top();
    
    // If there's an explicit variable in NEXT, verify it matches
    if (!stmt->children.empty()) {
        auto nextVar = stmt->children[0];
        if (nextVar->value != loopState.variable) {
            throw std::runtime_error("NEXT WITHOUT FOR");
        }
    }
    
    forLoopStack.pop();
    
    double currentValue = variables.getNumericVariable(loopState.variable);
    currentValue += loopState.stepValue;
    variables.setNumericVariable(loopState.variable, currentValue);
    
    DEBUG_PRINT("NEXT: " << loopState.variable << " = " << currentValue << ", forLoopStack size: " << forLoopStack.size());

    bool continueLoop = false;
    if (loopState.stepValue > 0) {
        continueLoop = currentValue <= loopState.endValue;
    } else {
        continueLoop = currentValue >= loopState.endValue;
    }
    
    if (continueLoop) {
        // Continue the loop - push the state back and jump to loop body
        forLoopStack.push(loopState);
        DEBUG_PRINT("  After NEXT push (continue loop), forLoopStack size: " << forLoopStack.size());
        if (loopState.returnStatementIndex >= 0) {
            // This case handles loops where the FOR statement has other statements on its line.
            // The return point is a specific statement index.
            if (this->currentLine == loopState.returnLine) {
                // This is a true same-line FOR..NEXT loop.
                // We are jumping to a statement on the same line.
                // The executeLine loop will not break, so we must adjust the index
                // to account for the loop's own increment.
                currentStatementIndex = loopState.returnStatementIndex - 1;
            } else {
                // The FOR and NEXT are on different lines.
                // We need to jump to the returnLine. This will cause executeLine to break
                // and the main loop to resume at the new line.
                currentLine = loopState.returnLine;
                currentStatementIndex = loopState.returnStatementIndex;
            }
        } else {
            // This case handles loops where FOR is the only statement on its line.
            gotoLine(loopState.returnLine);
        }
    }
    // If not continuing loop, just fall through to next statement
}

void AltairBasicInterpreter::executeProgram() {
    DEBUG_PRINT("Executing program...");
    if (program.empty()) {
        return;
    }
    
    running = true;
    stopExecution = false;
    
    // Only set currentLine to first line if we're not already positioned
    if (program.find(currentLine) == program.end()) {
        currentLine = program.begin()->first;
    }
    
    collectDataItems();
    
    while (running && !stopExecution) {
        DEBUG_PRINT("Program loop: currentLine=" << currentLine << ", currentStatementIndex=" << currentStatementIndex << ", callStack size: " << callStack.size() << ", forLoopStack size: " << forLoopStack.size());
        auto it = program.find(currentLine);
        if (it == program.end()) {
            break;
        }
        
        int originalLine = currentLine;
        executeLine(it->second.ast);
        
        // Only move to next line if currentLine wasn't changed by GOTO/GOSUB/NEXT
        if (currentLine == originalLine) {
            auto nextIt = program.upper_bound(currentLine);
            if (nextIt != program.end()) {
                currentLine = nextIt->first;
                currentStatementIndex = 0; // Reset statement index for new line
            } else {
                break;
            }
        }
    }
    
    DEBUG_PRINT("Program execution finished.");
    running = false;
}

void AltairBasicInterpreter::executeData(std::shared_ptr<ASTNode> stmt) {
    // DATA statements are processed during program execution setup
    // This method is called during execution but does nothing
}

void AltairBasicInterpreter::executeRead(std::shared_ptr<ASTNode> stmt) {
    if (stmt->children.empty()) return;
    
    auto varList = stmt->children[0];
    
    for (auto var : varList->children) {
        if (dataPointer >= dataItems.size()) {
            throw std::runtime_error("OUT OF DATA");
        }
        
        if (var->value.back() == '$') {
            // String variable - only simple variables supported for now
            variables.setStringVariable(var->value, dataItems[dataPointer]);
        } else {
            // Numeric variable
            try {
                double value = std::stod(dataItems[dataPointer]);
                
                if (var->type == NODE_ARRAY_ACCESS) {
                    std::vector<int> indices;
                    for (size_t i = 1; i < var->children.size(); i++) {
                        int index = static_cast<int>(evaluateExpression(var->children[i]));
                        indices.push_back(index);
                    }
                    variables.setArrayElement(var->value, indices, value);
                } else {
                    variables.setNumericVariable(var->value, value);
                }
            } catch (const std::exception&) {
                throw std::runtime_error("SYNTAX ERROR");
            }
        }
        
        dataPointer++;
    }
}

void AltairBasicInterpreter::executeRestore(std::shared_ptr<ASTNode> stmt) {
    dataPointer = 0;
}

void AltairBasicInterpreter::executeEnd(std::shared_ptr<ASTNode> stmt) {
    stopExecution = true;
}

void AltairBasicInterpreter::executeStop(std::shared_ptr<ASTNode> stmt) {
    std::cout << "BREAK IN " << currentLine << std::endl;
    stopExecution = true;
}

void AltairBasicInterpreter::executeOn(std::shared_ptr<ASTNode> stmt) {
    if (stmt->children.size() < 2) return;
    
    auto expr = stmt->children[0];
    auto action = stmt->children[1];
    
    int index = static_cast<int>(evaluateExpression(expr));
    if (index < 1 || index > static_cast<int>(action->children.size())) {
        return; // Out of range, do nothing
    }
    
    auto targetLine = action->children[index - 1]; // 1-based index
    int lineNumber = static_cast<int>(evaluateExpression(targetLine));
    
    if (action->keyword == KW_GOTO) {
        gotoLine(lineNumber);
    } else if (action->keyword == KW_GOSUB) {
        callStack.push(CallFrame(currentLine, currentStatementIndex));
        gotoLine(lineNumber);
    }
}

void AltairBasicInterpreter::executeList() {
    for (const auto& pair : program) {
        std::cout << pair.first << " ";
        // Reconstruct the original line text
        auto line = pair.second.ast;
        for (auto stmt : line->children) {
            printStatement(stmt);
        }
        std::cout << std::endl;
    }
}

void AltairBasicInterpreter::executeNew() {
    program.clear();
    variables.clearAll();
    dataItems.clear();
    dataPointer = 0;
    
    while (!callStack.empty()) {
        callStack.pop();
    }
    while (!forLoopStack.empty()) {
        forLoopStack.pop();
    }
}

void AltairBasicInterpreter::executeRun() {
    // Only clear stacks if we're not already running (to prevent clearing active GOSUB/FOR states)
    if (!running) {
        variables.clearAll();
        dataPointer = 0;
        
        while (!callStack.empty()) {
            callStack.pop();
        }
        while (!forLoopStack.empty()) {
            forLoopStack.pop();
        }
    }
    
    executeProgram();
}

void AltairBasicInterpreter::executeClear() {
    variables.clearAll();
}

void AltairBasicInterpreter::executeDim(std::shared_ptr<ASTNode> stmt) {
    for (auto dimDecl : stmt->children) {
        if (dimDecl->type == NODE_DIM_DECLARATION && dimDecl->children.size() >= 2) {
            auto arrayName = dimDecl->children[0];
            
            if (dimDecl->children.size() == 2) {
                // Single dimension: DIM A(10)
                auto sizeExpr = dimDecl->children[1];
                int size = static_cast<int>(evaluateExpression(sizeExpr));
                variables.dimArray(arrayName->value, size);
            } else {
                // Multi-dimensional: DIM A(10,20,5)
                std::vector<int> dimensions;
                for (size_t i = 1; i < dimDecl->children.size(); i++) {
                    int size = static_cast<int>(evaluateExpression(dimDecl->children[i]));
                    dimensions.push_back(size);
                }
                variables.dimArray(arrayName->value, dimensions);
            }
        }
    }
}

void AltairBasicInterpreter::executeDef(std::shared_ptr<ASTNode> stmt) {
    if (stmt->children.size() != 3) {
        throw std::runtime_error("SYNTAX ERROR");
    }
    
    std::string funcName = stmt->children[0]->value;
    std::string parameter = stmt->children[1]->value;
    auto body = stmt->children[2];
    
    userDefinedFunctions[funcName] = UserDefinedFunction(funcName, parameter, body);
}

double AltairBasicInterpreter::evaluateExpression(std::shared_ptr<ASTNode> expr) {
    DEBUG_PRINT("Evaluating expression of type: " << expr->type);
    switch (expr->type) {
        case NODE_NUMBER: {
            DEBUG_PRINT("  NODE_NUMBER: " << expr->value);
            return std::stod(expr->value);
        }
            
        case NODE_VARIABLE: {
            double value = variables.getNumericVariable(expr->value);
            DEBUG_PRINT("  NODE_VARIABLE: " << expr->value << " = " << value);
            return value;
        }
            
        case NODE_BINARY_OP:
            {
                // Check for string comparison
                auto left_node = expr->children[0];
                auto right_node = expr->children[1];

                bool isStringComparison = (left_node->type == NODE_STRING || 
                                           (left_node->type == NODE_VARIABLE && left_node->value.back() == '$') || 
                                           left_node->type == NODE_STRING_FUNCTION_CALL ||
                                           (left_node->type == NODE_ARRAY_ACCESS && left_node->children.size() >= 1 && left_node->children[0]->value.back() == '$') ||
                                           right_node->type == NODE_STRING || 
                                           (right_node->type == NODE_VARIABLE && right_node->value.back() == '$') || 
                                           right_node->type == NODE_STRING_FUNCTION_CALL ||
                                           (right_node->type == NODE_ARRAY_ACCESS && right_node->children.size() >= 1 && right_node->children[0]->value.back() == '$'));

                if (isStringComparison) {
                    std::string left_s = evaluateStringExpression(left_node);
                    std::string right_s = evaluateStringExpression(right_node);
                    DEBUG_PRINT("  NODE_BINARY_OP (string): "" << left_s << "" " << expr->operator_type << " "" << right_s << """);

                    switch (expr->operator_type) {
                        case OP_EQUAL: return (left_s == right_s) ? -1.0 : 0.0;
                        case OP_NOT_EQUAL: return (left_s != right_s) ? -1.0 : 0.0;
                        case OP_LESS: return (left_s < right_s) ? -1.0 : 0.0;
                        case OP_LESS_EQUAL: return (left_s <= right_s) ? -1.0 : 0.0;
                        case OP_GREATER: return (left_s > right_s) ? -1.0 : 0.0;
                        case OP_GREATER_EQUAL: return (left_s >= right_s) ? -1.0 : 0.0;
                        default: throw std::runtime_error("TYPE MISMATCH"); // Cannot do arithmetic on strings
                    }
                }

                double left = evaluateExpression(expr->children[0]);
                double right = evaluateExpression(expr->children[1]);
                DEBUG_PRINT("  NODE_BINARY_OP: " << left << " " << expr->operator_type << " " << right);
                
                switch (expr->operator_type) {
                    case OP_PLUS: return left + right;
                    case OP_MINUS: return left - right;
                    case OP_MULTIPLY: return left * right;
                    case OP_DIVIDE:
                        if (right == 0.0) throw std::runtime_error("DIVISION BY ZERO");
                        return left / right;
                    case OP_POWER: return std::pow(left, right);
                    case OP_EQUAL: return (left == right) ? -1.0 : 0.0;
                    case OP_NOT_EQUAL: return (left != right) ? -1.0 : 0.0;
                    case OP_LESS: return (left < right) ? -1.0 : 0.0;
                    case OP_LESS_EQUAL: return (left <= right) ? -1.0 : 0.0;
                    case OP_GREATER: return (left > right) ? -1.0 : 0.0;
                    case OP_GREATER_EQUAL: return (left >= right) ? -1.0 : 0.0;
                    default: throw std::runtime_error("SYNTAX ERROR");
                }
            }
            
        case NODE_UNARY_OP:
            {
                double operand = evaluateExpression(expr->children[0]);
                DEBUG_PRINT("  NODE_UNARY_OP: " << expr->value << " " << operand);
                if (expr->operator_type == OP_MINUS) {
                    return -operand;
                } else if (expr->value == "NOT") {
                    return (operand == 0.0) ? -1.0 : 0.0;
                }
                throw std::runtime_error("SYNTAX ERROR");
            }
            
        case NODE_FUNCTION_CALL:
            {
                DEBUG_PRINT("  NODE_FUNCTION_CALL: " << expr->value);
                
                // Handle special functions that take string arguments but return numbers
                std::string upperName = expr->value;
                std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
                
                if (upperName == "LEN" || upperName == "ASC" || upperName == "VAL") {
                    if (expr->children.size() != 1) {
                        throw std::runtime_error("SYNTAX ERROR");
                    }
                    
                    std::string strArg;
                    auto arg = expr->children[0];
                    
                    if (arg->type == NODE_STRING) {
                        strArg = arg->value;
                    } else if (arg->type == NODE_VARIABLE && arg->value.back() == '$') {
                        strArg = variables.getStringVariable(arg->value);
                    } else if (arg->type == NODE_STRING_FUNCTION_CALL) {
                        strArg = evaluateStringExpression(arg);
                    } else {
                        throw std::runtime_error("TYPE MISMATCH");
                    }
                    
                    if (upperName == "LEN") {
                        return MathFunctions::len(strArg);
                    } else if (upperName == "ASC") {
                        return MathFunctions::asc(strArg);
                    } else if (upperName == "VAL") {
                        return MathFunctions::val(strArg);
                    }
                }
                
                // Check for user-defined functions first
                if (userDefinedFunctions.find(expr->value) != userDefinedFunctions.end()) {
                    if (expr->children.size() != 1) {
                        throw std::runtime_error("SYNTAX ERROR");
                    }
                    
                    DEBUG_PRINT("Calling user-defined function: " << expr->value);
                    
                    auto& func = userDefinedFunctions[expr->value];
                    double argValue = evaluateExpression(expr->children[0]);
                    
                    DEBUG_PRINT("Setting parameter " << func.parameter << " = " << argValue);
                    
                    // Set parameter value
                    variables.setNumericVariable(func.parameter, argValue);
                    
                    // Evaluate function body
                    double result = evaluateExpression(func.body);
                    
                    return result;
                }
                
                std::vector<double> args;
                for (auto arg : expr->children) {
                    args.push_back(evaluateExpression(arg));
                }
                return MathFunctions::callFunction(expr->value, args);
            }
            
        case NODE_STRING_FUNCTION_CALL:
            {
                // String functions can't be used in numeric context
                throw std::runtime_error("TYPE MISMATCH");
            }
        case NODE_STRING:
            {
                // Strings can't be used in numeric context
                throw std::runtime_error("TYPE MISMATCH");
            }
            
        case NODE_ARRAY_ACCESS:
            {
                DEBUG_PRINT("  NODE_ARRAY_ACCESS: " << expr->children[0]->value);
                if (expr->children.size() < 2) {
                    throw std::runtime_error("SYNTAX ERROR");
                }
                auto arrayName = expr->children[0];
                
                // Check if this is actually a user-defined function call
                if (userDefinedFunctions.find(arrayName->value) != userDefinedFunctions.end()) {
                    if (expr->children.size() != 2) {
                        throw std::runtime_error("SYNTAX ERROR");
                    }
                    
                    auto& func = userDefinedFunctions[arrayName->value];
                    double argValue = evaluateExpression(expr->children[1]);
                    
                    // Set parameter value
                    variables.setNumericVariable(func.parameter, argValue);
                    
                    // Evaluate function body
                    double result = evaluateExpression(func.body);
                    
                    return result;
                }
                
                // Check if this is a string array - if so, throw TYPE MISMATCH
                if (arrayName->value.back() == '$') {
                    throw std::runtime_error("TYPE MISMATCH");
                }
                
                if (expr->children.size() == 2) {
                    // Single dimension: A(1)
                    auto indexExpr = expr->children[1];
                    int index = static_cast<int>(evaluateExpression(indexExpr));
                    return variables.getArrayElement(arrayName->value, index);
                } else {
                    // Multi-dimensional: A(1,2,3)
                    std::vector<int> indices;
                    for (size_t i = 1; i < expr->children.size(); i++) {
                        int index = static_cast<int>(evaluateExpression(expr->children[i]));
                        indices.push_back(index);
                    }
                    return variables.getArrayElement(arrayName->value, indices);
                }
            }
            
        default:
            throw std::runtime_error("SYNTAX ERROR");
    }
}

std::string AltairBasicInterpreter::evaluateStringExpression(std::shared_ptr<ASTNode> expr) {
    switch (expr->type) {
        case NODE_STRING:
            return expr->value;
            
        case NODE_VARIABLE:
            // String variable access: A$, G2$, etc.
            if (expr->value.back() == '$') {
                return variables.getStringVariable(expr->value);
            } else {
                throw std::runtime_error("TYPE MISMATCH");
            }
            
        case NODE_ARRAY_ACCESS:
            {
                if (expr->children.size() < 2) {
                    throw std::runtime_error("SYNTAX ERROR");
                }
                auto arrayName = expr->children[0];
                
                // Check if this is a string array
                if (arrayName->value.back() == '$') {
                    if (expr->children.size() == 2) {
                        // Single dimension: A$(1)
                        auto indexExpr = expr->children[1];
                        int index = static_cast<int>(evaluateExpression(indexExpr));
                        return variables.getStringArrayElement(arrayName->value, index);
                    } else {
                        // Multi-dimensional: A$(1,2,3)
                        std::vector<int> indices;
                        for (size_t i = 1; i < expr->children.size(); i++) {
                            int index = static_cast<int>(evaluateExpression(expr->children[i]));
                            indices.push_back(index);
                        }
                        return variables.getStringArrayElement(arrayName->value, indices);
                    }
                } else {
                    throw std::runtime_error("TYPE MISMATCH");
                }
            }
            
        case NODE_STRING_FUNCTION_CALL:
            {
                std::vector<double> numArgs;
                std::vector<std::string> strArgs;
                
                for (auto arg : expr->children) {
                    if (arg->type == NODE_STRING) {
                        strArgs.push_back(arg->value);
                    } else if (arg->type == NODE_VARIABLE && arg->value.back() == '$') {
                        strArgs.push_back(variables.getStringVariable(arg->value));
                    } else if (arg->type == NODE_STRING_FUNCTION_CALL) {
                        strArgs.push_back(evaluateStringExpression(arg));
                    } else {
                        numArgs.push_back(evaluateExpression(arg));
                    }
                }
                
                return MathFunctions::callStringFunction(expr->value, numArgs, strArgs);
            }
            
        default:
            throw std::runtime_error("TYPE MISMATCH");
    }
}

int AltairBasicInterpreter::getNextLineNumber(int currentLineNum) {
    auto nextIt = program.upper_bound(currentLineNum);
    if (nextIt != program.end()) {
        return nextIt->first;
    }
    return currentLineNum; // If no next line, return current (end of program)
}

void AltairBasicInterpreter::findMatchingNext(int forLineNum) {
    int forCount = 1; // We've seen one FOR (the current one)
    
    // Start searching from the line after the FOR
    auto startIt = program.upper_bound(forLineNum);
    
    for (auto it = startIt; it != program.end(); ++it) {
        auto line = it->second.ast;
        for (auto stmt : line->children) {
            if (stmt->keyword == KW_FOR) {
                forCount++;
            } else if (stmt->keyword == KW_NEXT) {
                forCount--;
                if (forCount == 0) {
                    // Found matching NEXT, jump to the line after it
                    int nextLineAfterNext = getNextLineNumber(it->first);
                    gotoLine(nextLineAfterNext);
                    return;
                }
            }
        }
    }
    
    // If no matching NEXT found, this is an error but we'll just end execution
    stopExecution = true;
}

void AltairBasicInterpreter::gotoLine(int lineNumber) {
    if (program.find(lineNumber) == program.end()) {
        throw std::runtime_error("UNDEFINED LINE NUMBER");
    }
    currentLine = lineNumber;
    currentStatementIndex = 0;
}

void AltairBasicInterpreter::gotoStatement(int lineNumber, int statementIndex) {
    if (program.find(lineNumber) == program.end()) {
        throw std::runtime_error("UNDEFINED LINE NUMBER");
    }
    
    auto it = program.find(lineNumber);
    if (statementIndex >= static_cast<int>(it->second.ast->children.size()) || statementIndex < 0) {
        throw std::runtime_error("SYNTAX ERROR");
    }
    
    currentLine = lineNumber;
    
    // Execute from the specified statement index to end of line
    auto line = it->second.ast;
    for (currentStatementIndex = statementIndex; currentStatementIndex < static_cast<int>(line->children.size()); currentStatementIndex++) {
        if (stopExecution) break;
        executeStatement(line->children[currentStatementIndex]);
    }
}

void AltairBasicInterpreter::cleanupForLoopStackOnGoto(int fromLine, int toLine) {
    // Collect all NEXT statements that are being jumped over
    std::set<std::string> jumpedOverVariables;
    
    int startLine = std::min(fromLine, toLine);
    int endLine = std::max(fromLine, toLine);
    
    // If jumping backward, check what NEXT statements we're jumping over
    if (toLine < fromLine) {
        // Scan from toLine+1 to fromLine to find NEXT statements being jumped over
        for (auto it = program.upper_bound(toLine); it != program.end() && it->first <= fromLine; ++it) {
            auto line = it->second.ast;
            for (auto stmt : line->children) {
                if (stmt->keyword == KW_NEXT) {
                    // This NEXT is being jumped over backwards - find its variable
                    if (!stmt->children.empty()) {
                        jumpedOverVariables.insert(stmt->children[0]->value);
                    } else {
                        // NEXT without explicit variable - matches most recent FOR
                        if (!forLoopStack.empty()) {
                            jumpedOverVariables.insert(forLoopStack.top().variable);
                        }
                    }
                }
            }
        }
    } else {
        // Jumping forward - scan from fromLine+1 to toLine-1 to find NEXT statements being jumped over
        for (auto it = program.upper_bound(fromLine); it != program.end() && it->first < toLine; ++it) {
            auto line = it->second.ast;
            for (auto stmt : line->children) {
                if (stmt->keyword == KW_NEXT) {
                    // This NEXT is being jumped over - find its variable
                    if (!stmt->children.empty()) {
                        jumpedOverVariables.insert(stmt->children[0]->value);
                    } else {
                        // NEXT without explicit variable - matches most recent FOR
                        if (!forLoopStack.empty()) {
                            jumpedOverVariables.insert(forLoopStack.top().variable);
                        }
                    }
                }
            }
        }
    }
    
    // Remove FOR loop states for variables whose NEXT statements are jumped over
    std::stack<ForLoopState> tempStack;
    while (!forLoopStack.empty()) {
        auto loopState = forLoopStack.top();
        forLoopStack.pop();
        
        if (jumpedOverVariables.find(loopState.variable) == jumpedOverVariables.end()) {
            // This loop's NEXT is not jumped over, keep it
            tempStack.push(loopState);
        }
        // Otherwise, this loop is exited by the GOTO, so don't keep it
    }
    
    // Restore the remaining loop states
    while (!tempStack.empty()) {
        forLoopStack.push(tempStack.top());
        tempStack.pop();
    }
}

void AltairBasicInterpreter::collectDataItems() {
    dataItems.clear();
    dataPointer = 0;
    
    for (const auto& pair : program) {
        auto line = pair.second.ast;
        for (auto stmt : line->children) {
            if (stmt->keyword == KW_DATA) {
                for (auto data : stmt->children) {
                    dataItems.push_back(data->value);
                }
            }
        }
    }
}

std::string AltairBasicInterpreter::formatNumber(double value) {
    std::ostringstream oss;
    
    // Authentic BASIC number formatting
    if (value >= 0) {
        oss << " "; // Leading space for positive numbers
    }
    
    if (value == static_cast<int>(value) && std::abs(value) < 1e6) {
        oss << static_cast<int>(value);
    } else {
        // Use E notation for very large/small numbers like authentic BASIC
        if (std::abs(value) >= 1e6 || (std::abs(value) < 1e-3 && value != 0)) {
            oss << std::scientific << std::setprecision(5) << value;
        } else {
            oss << std::fixed << std::setprecision(6) << value;
            std::string result = oss.str();
            // Remove trailing zeros
            result.erase(result.find_last_not_of('0') + 1, std::string::npos);
            result.erase(result.find_last_not_of('.') + 1, std::string::npos);
            oss.str("");
            oss << result;
        }
    }
    
    // Add trailing space for authentic BASIC formatting
    oss << " ";
    
    return oss.str();
}

void AltairBasicInterpreter::printTabs(int count) {
    for (int i = 0; i < count; ++i) {
        std::cout << " ";
    }
}

void AltairBasicInterpreter::printStatement(std::shared_ptr<ASTNode> stmt) {
    // Simple reconstruction of statement text for LIST command
    switch (stmt->keyword) {
        case KW_PRINT:
            std::cout << "PRINT";
            for (auto child : stmt->children) {
                if (child->type == NODE_STRING && child->value != "," && child->value != ";") {
                    std::cout << " "" << child->value << """;
                } else if (child->type == NODE_STRING) {
                    std::cout << child->value;
                } else {
                    std::cout << " [EXPR]";
                }
            }
            break;
        case KW_INPUT:
            std::cout << "INPUT";
            break;
        case KW_LET:
            std::cout << "LET";
            break;
        case KW_IF:
            std::cout << "IF [CONDITION] THEN [ACTION]";
            break;
        case KW_FOR:
            std::cout << "FOR [VAR]=[START] TO [END]";
            break;
        case KW_GOTO:
            std::cout << "GOTO";
            if (!stmt->children.empty()) {
                std::cout << " " << stmt->children[0]->value;
            }
            break;
        case KW_GOSUB:
            std::cout << "GOSUB";
            if (!stmt->children.empty()) {
                std::cout << " " << stmt->children[0]->value;
            }
            break;
        case KW_RETURN:
            std::cout << "RETURN";
            break;
        case KW_REM:
            std::cout << "REM";
            if (!stmt->children.empty()) {
                std::cout << " " << stmt->children[0]->value;
            }
            break;
        default:
            std::cout << "[STATEMENT]";
    }
}
