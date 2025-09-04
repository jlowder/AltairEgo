#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "interpreter.h"

int main(int argc, char* argv[]) {
    AltairBasicInterpreter interpreter;

    if (argc > 1) {
        // File mode
        std::ifstream file(argv[1]);
        if (!file) {
            std::cerr << "CAN'T OPEN " << argv[1] << std::endl;
            return 1;
        }
        std::string line;
        while (std::getline(file, line)) {
            try {
                interpreter.processLine(line);
            } catch (const std::exception& e) {
                std::cerr << "ERROR IN " << line << ": " << e.what() << std::endl;
            }
        }
        try {
            interpreter.executeRun();
        } catch (const std::exception& e) {
            std::cerr << "RUNTIME ERROR: " << e.what() << std::endl;
        }
    } else {
        // Interactive mode
	std::cout << "Altair Ego: Emulating Altair BASIC 32K Rev. 3.2" << std::endl;
        std::cout << "OK" << std::endl;

        std::string line;
        while (true) {
            std::cout << "";
            if (!std::getline(std::cin, line)) {
                break;
            }

            if (line.empty()) {
                continue;
            }

            try {
                interpreter.processLine(line);
            } catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
            }
        }
    }
    
    return 0;
}
