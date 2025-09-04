#include "functions.h"
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>

double MathFunctions::abs(double x) {
    return std::abs(x);
}

double MathFunctions::int_func(double x) {
    return std::floor(x);
}

double MathFunctions::sqr(double x) {
    if (x < 0) {
        throw std::runtime_error("ILLEGAL FUNCTION CALL");
    }
    return std::sqrt(x);
}

double MathFunctions::sin_func(double x) {
    return std::sin(x);
}

double MathFunctions::tab(double x) {
    // TAB function for PRINT statement formatting
    int spaces = static_cast<int>(x);
    if (spaces < 0) spaces = 0;
    if (spaces > 255) spaces = 255;
    return spaces;
}

double MathFunctions::usr(double x) {
    // USR function for machine language calls
    // In authentic Altair BASIC, this would call machine language
    // For this implementation, return 0
    return 0.0;
}

double MathFunctions::cos_func(double x) {
    return std::cos(x);
}

double MathFunctions::atn(double x) {
    return std::atan(x);
}

double MathFunctions::exp_func(double x) {
    return std::exp(x);
}

double MathFunctions::log_func(double x) {
    if (x <= 0) {
        throw std::runtime_error("ILLEGAL FUNCTION CALL");
    }
    return std::log(x);
}

double MathFunctions::sgn(double x) {
    if (x > 0) return 1.0;
    if (x < 0) return -1.0;
    return 0.0;
}

double MathFunctions::rnd() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0.0, 1.0);
    static double lastRandom = 0.0;
    
    lastRandom = dis(gen);
    return lastRandom;
}

double MathFunctions::rnd(double x) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0.0, 1.0);
    static double lastRandom = 0.0;
    
    if (x > 0) {
        // RND(positive) - return new random number
        lastRandom = dis(gen);
        return lastRandom;
    } else if (x == 0) {
        // RND(0) - return last random number
        return lastRandom;
    } else {
        // RND(negative) - seed the generator
        gen.seed(static_cast<unsigned int>(-x));
        lastRandom = dis(gen);
        return lastRandom;
    }
}

// String functions
std::string MathFunctions::chr_func(double x) {
    int ascii = static_cast<int>(x);
    if (ascii < 0 || ascii > 255) {
        throw std::runtime_error("ILLEGAL FUNCTION CALL");
    }
    return std::string(1, static_cast<char>(ascii));
}

double MathFunctions::asc(const std::string& s) {
    if (s.empty()) {
        throw std::runtime_error("ILLEGAL FUNCTION CALL");
    }
    return static_cast<double>(static_cast<unsigned char>(s[0]));
}

double MathFunctions::len(const std::string& s) {
    return static_cast<double>(s.length());
}

std::string MathFunctions::left_func(const std::string& s, double n) {
    int count = static_cast<int>(n);
    if (count < 0) count = 0;
    if (count > static_cast<int>(s.length())) count = s.length();
    return s.substr(0, count);
}

std::string MathFunctions::right_func(const std::string& s, double n) {
    int count = static_cast<int>(n);
    if (count < 0) count = 0;
    if (count > static_cast<int>(s.length())) count = s.length();
    return s.substr(s.length() - count, count);
}

std::string MathFunctions::mid_func(const std::string& s, double start, double length) {
    int startPos = static_cast<int>(start) - 1; // BASIC uses 1-based indexing
    int len = static_cast<int>(length);
    if (startPos < 0) startPos = 0;
    if (len < 0) len = 0;
    if (startPos >= static_cast<int>(s.length())) return "";
    return s.substr(startPos, len);
}

std::string MathFunctions::str_func(double x) {
    std::ostringstream oss;
    if (x == static_cast<int>(x) && std::abs(x) < 1e6) {
        oss << static_cast<int>(x);
    } else {
        oss << std::fixed << std::setprecision(6) << x;
        std::string result = oss.str();
        // Remove trailing zeros
        result.erase(result.find_last_not_of('0') + 1, std::string::npos);
        result.erase(result.find_last_not_of('.') + 1, std::string::npos);
    }
    return oss.str();
}

double MathFunctions::val(const std::string& s) {
    try {
        return std::stod(s);
    } catch (const std::exception&) {
        return 0.0; // Invalid strings return 0 in BASIC
    }
}

bool MathFunctions::isMathFunction(const std::string& name) {
    std::string upperName = name;
    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
    
    return upperName == "ABS" || upperName == "INT" || upperName == "SQR" ||
           upperName == "SIN" || upperName == "COS" || upperName == "ATN" ||
           upperName == "EXP" || upperName == "LOG" || upperName == "SGN" ||
           upperName == "TAB" || upperName == "USR" || upperName == "RND" ||
           upperName == "ASC" || upperName == "LEN" || upperName == "VAL";
}

bool MathFunctions::isStringFunction(const std::string& name) {
    std::string upperName = name;
    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
    
    return upperName == "CHR$" || upperName == "LEFT$" || upperName == "RIGHT$" ||
           upperName == "MID$" || upperName == "STR$";
}

double MathFunctions::callFunction(const std::string& name, const std::vector<double>& args) {
    std::string upperName = name;
    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
    
    if (upperName == "ABS") {
        if (args.size() != 1) throw std::runtime_error("SYNTAX ERROR");
        return abs(args[0]);
    } else if (upperName == "INT") {
        if (args.size() != 1) throw std::runtime_error("SYNTAX ERROR");
        return int_func(args[0]);
    } else if (upperName == "SQR") {
        if (args.size() != 1) throw std::runtime_error("SYNTAX ERROR");
        return sqr(args[0]);
    } else if (upperName == "SIN") {
        if (args.size() != 1) throw std::runtime_error("SYNTAX ERROR");
        return sin_func(args[0]);
    } else if (upperName == "COS") {
        if (args.size() != 1) throw std::runtime_error("SYNTAX ERROR");
        return cos_func(args[0]);
    } else if (upperName == "ATN") {
        if (args.size() != 1) throw std::runtime_error("SYNTAX ERROR");
        return atn(args[0]);
    } else if (upperName == "EXP") {
        if (args.size() != 1) throw std::runtime_error("SYNTAX ERROR");
        return exp_func(args[0]);
    } else if (upperName == "LOG") {
        if (args.size() != 1) throw std::runtime_error("SYNTAX ERROR");
        return log_func(args[0]);
    } else if (upperName == "SGN") {
        if (args.size() != 1) throw std::runtime_error("SYNTAX ERROR");
        return sgn(args[0]);
    } else if (upperName == "TAB") {
        if (args.size() != 1) throw std::runtime_error("SYNTAX ERROR");
        return tab(args[0]);
    } else if (upperName == "USR") {
        if (args.size() != 1) throw std::runtime_error("SYNTAX ERROR");
        return usr(args[0]);
    } else if (upperName == "RND") {
        if (args.size() == 0) {
            return rnd();
        } else if (args.size() == 1) {
            return rnd(args[0]);
        } else {
            throw std::runtime_error("SYNTAX ERROR");
        }
    } else if (upperName == "ASC") {
        // Handled in interpreter for string arguments
        throw std::runtime_error("UNDEFINED FUNCTION");
    } else if (upperName == "LEN") {
        // Handled in interpreter for string arguments
        throw std::runtime_error("UNDEFINED FUNCTION");
    } else if (upperName == "VAL") {
        // Handled in interpreter for string arguments
        throw std::runtime_error("UNDEFINED FUNCTION");
    }
    
    throw std::runtime_error("UNDEFINED FUNCTION");
}

std::string MathFunctions::callStringFunction(const std::string& name, const std::vector<double>& numArgs, const std::vector<std::string>& strArgs) {
    std::string upperName = name;
    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
    
    if (upperName == "CHR$") {
        if (numArgs.size() != 1) throw std::runtime_error("SYNTAX ERROR");
        return chr_func(numArgs[0]);
    } else if (upperName == "LEFT$") {
        if (strArgs.size() != 1 || numArgs.size() != 1) throw std::runtime_error("SYNTAX ERROR");
        return left_func(strArgs[0], numArgs[0]);
    } else if (upperName == "RIGHT$") {
        if (strArgs.size() != 1 || numArgs.size() != 1) throw std::runtime_error("SYNTAX ERROR");
        return right_func(strArgs[0], numArgs[0]);
    } else if (upperName == "MID$") {
        if (strArgs.size() != 1 || numArgs.size() != 2) throw std::runtime_error("SYNTAX ERROR");
        return mid_func(strArgs[0], numArgs[0], numArgs[1]);
    } else if (upperName == "STR$") {
        if (numArgs.size() != 1) throw std::runtime_error("SYNTAX ERROR");
        return str_func(numArgs[0]);
    }
    
    throw std::runtime_error("UNDEFINED FUNCTION");
}
