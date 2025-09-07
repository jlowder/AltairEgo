#include "variable.h"
#include <stdexcept>
#include <cctype>

VariableManager::VariableManager() {}

void VariableManager::setNumericVariable(const std::string& name, double value) {
    std::string normalizedName = normalizeVariableName(name);
    if (!isValidVariableName(normalizedName)) {
        throw std::runtime_error("ILLEGAL VARIABLE NAME");
    }
    numericVariables[normalizedName] = value;
}

double VariableManager::getNumericVariable(const std::string& name) {
    std::string normalizedName = normalizeVariableName(name);
    auto it = numericVariables.find(normalizedName);
    if (it != numericVariables.end()) {
        return it->second;
    }
    return 0.0; // Uninitialized variables default to 0
}

bool VariableManager::isNumericVariable(const std::string& name) {
    std::string normalizedName = normalizeVariableName(name);
    return numericVariables.find(normalizedName) != numericVariables.end();
}

bool VariableManager::hasVariable(const std::string& name) {
    std::string normalizedName = normalizeVariableName(name);
    return numericVariables.find(normalizedName) != numericVariables.end();
}

void VariableManager::setStringVariable(const std::string& name, const std::string& value) {
    std::string normalizedName = normalizeVariableName(name);
    if (!isValidVariableName(normalizedName)) {
        throw std::runtime_error("ILLEGAL VARIABLE NAME");
    }
    stringVariables[normalizedName] = value;
}

std::string VariableManager::getStringVariable(const std::string& name) {
    std::string normalizedName = normalizeVariableName(name);
    auto it = stringVariables.find(normalizedName);
    if (it != stringVariables.end()) {
        return it->second;
    }
    return ""; // Uninitialized string variables default to empty string
}

bool VariableManager::isStringVariable(const std::string& name) {
    std::string normalizedName = normalizeVariableName(name);
    return stringVariables.find(normalizedName) != stringVariables.end();
}

void VariableManager::dimArray(const std::string& name, int size) {
    std::string normalizedName = normalizeVariableName(name);
    if (!isValidVariableName(normalizedName)) {
        throw std::runtime_error("ILLEGAL VARIABLE NAME");
    }
    if (size < 0) {
        throw std::runtime_error("ILLEGAL FUNCTION CALL");
    }
    
    if (normalizedName.back() == '$') {
        // String array
        stringArrays[normalizedName] = std::vector<std::string>(size + 1, "");
    } else {
        // Numeric array
        arrays[normalizedName] = std::vector<double>(size + 1, 0.0);
    }
    arrayDimensions[normalizedName] = {size + 1};
}

void VariableManager::dimArray(const std::string& name, const std::vector<int>& dimensions) {
    std::string normalizedName = normalizeVariableName(name);
    if (!isValidVariableName(normalizedName)) {
        throw std::runtime_error("ILLEGAL VARIABLE NAME");
    }
    
    // Calculate total size for multi-dimensional array
    int totalSize = 1;
    std::vector<int> adjustedDims;
    for (int dim : dimensions) {
        if (dim < 0) {
            throw std::runtime_error("ILLEGAL FUNCTION CALL");
        }
        adjustedDims.push_back(dim + 1); // BASIC arrays include 0 index
        totalSize *= (dim + 1);
    }
    
    if (normalizedName.back() == '$') {
        // String array
        stringArrays[normalizedName] = std::vector<std::string>(totalSize, "");
    } else {
        // Numeric array
        arrays[normalizedName] = std::vector<double>(totalSize, 0.0);
    }
    arrayDimensions[normalizedName] = adjustedDims;
}

void VariableManager::setArrayElement(const std::string& name, int index, double value) {
    std::string normalizedName = normalizeVariableName(name);
    auto it = arrays.find(normalizedName);
    if (it == arrays.end()) {
        // Auto-dimension with default size 10
        dimArray(normalizedName, 10);
        it = arrays.find(normalizedName);
    }
    
    if (index < 0 || index >= static_cast<int>(it->second.size())) {
        throw std::runtime_error("SUBSCRIPT OUT OF RANGE");
    }
    
    it->second[index] = value;
}

double VariableManager::getArrayElement(const std::string& name, int index) {
    std::string normalizedName = normalizeVariableName(name);
    auto it = arrays.find(normalizedName);
    if (it == arrays.end()) {
        // Auto-dimension with default size 10
        dimArray(normalizedName, 10);
        it = arrays.find(normalizedName);
    }
    
    if (index < 0 || index >= static_cast<int>(it->second.size())) {
        throw std::runtime_error("SUBSCRIPT OUT OF RANGE");
    }
    
    return it->second[index];
}

bool VariableManager::isArray(const std::string& name) {
    std::string normalizedName = normalizeVariableName(name);
    return arrays.find(normalizedName) != arrays.end();
}

void VariableManager::setArrayElement(const std::string& name, const std::vector<int>& indices, double value) {
    std::string normalizedName = normalizeVariableName(name);
    auto arrayIt = arrays.find(normalizedName);
    auto dimIt = arrayDimensions.find(normalizedName);
    
    if (arrayIt == arrays.end() || dimIt == arrayDimensions.end()) {
        throw std::runtime_error("SUBSCRIPT OUT OF RANGE");
    }
    
    // Validate indices and calculate linear index
    if (indices.size() != dimIt->second.size()) {
        throw std::runtime_error("SUBSCRIPT OUT OF RANGE");
    }
    
    int linearIndex = 0;
    int multiplier = 1;
    for (int i = indices.size() - 1; i >= 0; i--) {
        if (indices[i] < 0 || indices[i] >= dimIt->second[i]) {
            throw std::runtime_error("SUBSCRIPT OUT OF RANGE");
        }
        linearIndex += indices[i] * multiplier;
        multiplier *= dimIt->second[i];
    }
    
    arrayIt->second[linearIndex] = value;
}

double VariableManager::getArrayElement(const std::string& name, const std::vector<int>& indices) {
    std::string normalizedName = normalizeVariableName(name);
    auto arrayIt = arrays.find(normalizedName);
    auto dimIt = arrayDimensions.find(normalizedName);
    
    if (arrayIt == arrays.end() || dimIt == arrayDimensions.end()) {
        throw std::runtime_error("SUBSCRIPT OUT OF RANGE");
    }
    
    // Validate indices and calculate linear index
    if (indices.size() != dimIt->second.size()) {
        throw std::runtime_error("SUBSCRIPT OUT OF RANGE");
    }
    
    int linearIndex = 0;
    int multiplier = 1;
    for (int i = indices.size() - 1; i >= 0; i--) {
        if (indices[i] < 0 || indices[i] >= dimIt->second[i]) {
            throw std::runtime_error("SUBSCRIPT OUT OF RANGE");
        }
        linearIndex += indices[i] * multiplier;
        multiplier *= dimIt->second[i];
    }
    
    return arrayIt->second[linearIndex];
}

void VariableManager::clearAll() {
    numericVariables.clear();
    stringVariables.clear();
    arrays.clear();
    stringArrays.clear();
    arrayDimensions.clear();
}

bool VariableManager::isValidVariableName(const std::string& name) {
    if (name.empty()) {
        return false;
    }
    
    // Must start with a letter
    if (!std::isalpha(name[0])) {
        return false;
    }
    
    // In Altair BASIC 4K, variables are single letter or letter+digit
    if (name.length() == 1) {
        return std::isalpha(name[0]);
    } else if (name.length() == 2) {
        // Could be A9 (numeric) or A$ (string)
        if (name[1] == '$') {
            return std::isalpha(name[0]); // A$ to Z$
        } else {
            return std::isalpha(name[0]) && std::isdigit(name[1]); // A0 to Z9
        }
    } else if (name.length() == 3 && name[2] == '$') {
        // String variable (A0$ to Z9$)
        return std::isalpha(name[0]) && std::isdigit(name[1]);
    }
    
    return false;
}

void VariableManager::setStringArrayElement(const std::string& name, int index, const std::string& value) {
    std::string normalizedName = normalizeVariableName(name);
    auto it = stringArrays.find(normalizedName);
    
    if (it == stringArrays.end()) {
        // Array doesn't exist, create it with default size
        stringArrays[normalizedName] = std::vector<std::string>(11, ""); // Default size 0-10
        arrayDimensions[normalizedName] = {11};
    }
    
    if (index < 0 || index >= static_cast<int>(it->second.size())) {
        throw std::runtime_error("SUBSCRIPT OUT OF RANGE");
    }
    
    stringArrays[normalizedName][index] = value;
}

void VariableManager::setStringArrayElement(const std::string& name, const std::vector<int>& indices, const std::string& value) {
    std::string normalizedName = normalizeVariableName(name);
    auto arrayIt = stringArrays.find(normalizedName);
    auto dimIt = arrayDimensions.find(normalizedName);
    
    if (arrayIt == stringArrays.end() || dimIt == arrayDimensions.end()) {
        throw std::runtime_error("SUBSCRIPT OUT OF RANGE");
    }
    
    if (indices.size() != dimIt->second.size()) {
        throw std::runtime_error("SUBSCRIPT OUT OF RANGE");
    }
    
    int flatIndex = 0;
    int multiplier = 1;
    
    for (int i = indices.size() - 1; i >= 0; i--) {
        if (indices[i] < 0 || indices[i] >= dimIt->second[i]) {
            throw std::runtime_error("SUBSCRIPT OUT OF RANGE");
        }
        flatIndex += indices[i] * multiplier;
        multiplier *= dimIt->second[i];
    }
    
    arrayIt->second[flatIndex] = value;
}

std::string VariableManager::getStringArrayElement(const std::string& name, int index) {
    std::string normalizedName = normalizeVariableName(name);
    auto it = stringArrays.find(normalizedName);
    
    if (it == stringArrays.end()) {
        // Array doesn't exist, create it with default size
        stringArrays[normalizedName] = std::vector<std::string>(11, ""); // Default size 0-10
        arrayDimensions[normalizedName] = {11};
        return "";
    }
    
    if (index < 0 || index >= static_cast<int>(it->second.size())) {
        throw std::runtime_error("SUBSCRIPT OUT OF RANGE");
    }
    
    return it->second[index];
}

std::string VariableManager::getStringArrayElement(const std::string& name, const std::vector<int>& indices) {
    std::string normalizedName = normalizeVariableName(name);
    auto arrayIt = stringArrays.find(normalizedName);
    auto dimIt = arrayDimensions.find(normalizedName);
    
    if (arrayIt == stringArrays.end() || dimIt == arrayDimensions.end()) {
        throw std::runtime_error("SUBSCRIPT OUT OF RANGE");
    }
    
    if (indices.size() != dimIt->second.size()) {
        throw std::runtime_error("SUBSCRIPT OUT OF RANGE");
    }
    
    int flatIndex = 0;
    int multiplier = 1;
    
    for (int i = indices.size() - 1; i >= 0; i--) {
        if (indices[i] < 0 || indices[i] >= dimIt->second[i]) {
            throw std::runtime_error("SUBSCRIPT OUT OF RANGE");
        }
        flatIndex += indices[i] * multiplier;
        multiplier *= dimIt->second[i];
    }
    
    return arrayIt->second[flatIndex];
}

bool VariableManager::isStringArray(const std::string& name) {
    std::string normalizedName = normalizeVariableName(name);
    return stringArrays.find(normalizedName) != stringArrays.end();
}

std::string VariableManager::normalizeVariableName(const std::string& name) {
    std::string normalized;
    for (char c : name) {
        normalized += std::toupper(c);
    }
    return normalized;
}
