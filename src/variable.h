#ifndef VARIABLE_H
#define VARIABLE_H

#include <string>
#include <map>
#include <vector>

class VariableManager {
private:
    std::map<std::string, double> numericVariables;
    std::map<std::string, std::string> stringVariables;
    std::map<std::string, std::vector<double>> arrays;
    std::map<std::string, std::vector<std::string>> stringArrays;
    std::map<std::string, std::vector<int>> arrayDimensions;
    
public:
    VariableManager();
    
    // Numeric variable operations
    void setNumericVariable(const std::string& name, double value);
    double getNumericVariable(const std::string& name);
    bool isNumericVariable(const std::string& name);
    bool hasVariable(const std::string& name);
    
    // String variable operations
    void setStringVariable(const std::string& name, const std::string& value);
    std::string getStringVariable(const std::string& name);
    bool isStringVariable(const std::string& name);
    
    // Array operations
    void dimArray(const std::string& name, int size);
    void dimArray(const std::string& name, const std::vector<int>& dimensions);
    void setArrayElement(const std::string& name, int index, double value);
    void setArrayElement(const std::string& name, const std::vector<int>& indices, double value);
    double getArrayElement(const std::string& name, int index);
    double getArrayElement(const std::string& name, const std::vector<int>& indices);
    bool isArray(const std::string& name);
    
    // String array operations
    void setStringArrayElement(const std::string& name, int index, const std::string& value);
    void setStringArrayElement(const std::string& name, const std::vector<int>& indices, const std::string& value);
    std::string getStringArrayElement(const std::string& name, int index);
    std::string getStringArrayElement(const std::string& name, const std::vector<int>& indices);
    bool isStringArray(const std::string& name);
    
    // Utility
    void clearAll();
    bool isValidVariableName(const std::string& name);
    std::string normalizeVariableName(const std::string& name);
};

#endif
