#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <vector>

class MathFunctions {
public:
    static double abs(double x);
    static double int_func(double x);
    static double sqr(double x);
    static double sin_func(double x);
    static double cos_func(double x);
    static double atn(double x);
    static double exp_func(double x);
    static double log_func(double x);
    static double sgn(double x);
    static double tab(double x);
    static double usr(double x);
    static double rnd();
    static double rnd(double x);
    
    // String functions
    static std::string chr_func(double x);
    static double asc(const std::string& s);
    static double len(const std::string& s);
    static std::string left_func(const std::string& s, double n);
    static std::string right_func(const std::string& s, double n);
    static std::string mid_func(const std::string& s, double start, double length);
    static std::string str_func(double x);
    static double val(const std::string& s);
    
    static bool isMathFunction(const std::string& name);
    static bool isStringFunction(const std::string& name);
    static double callFunction(const std::string& name, const std::vector<double>& args);
    static std::string callStringFunction(const std::string& name, const std::vector<double>& numArgs, const std::vector<std::string>& strArgs);
};

#endif
