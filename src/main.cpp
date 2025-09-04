#include "interpreter.h"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/emscripten.h>

EM_JS(void, flush_output_to_js, (const char* text), {
    const outputElement = document.getElementById('output');
    if (outputElement) {
        outputElement.innerHTML += UTF8ToString(text);
        outputElement.scrollTop = outputElement.scrollHeight;
    }
});

EM_JS(void, await_input_from_js, (), {
  return Asyncify.handleAsync(async () => {
    window.isAwaitingInput = true;
    await window.awaitInputPromise;
    window.isAwaitingInput = false;
  });
});

// Global interpreter instance
AltairBasicInterpreter interpreter;
// Stringstream to capture output
std::stringstream output_buffer;
// To hold the result to be passed to JS
static std::string result_string;

std::stringstream output_buffer;

// New buffer for input
static char input_buffer[256];

extern "C" {

EMSCRIPTEN_KEEPALIVE
const char* get_input_buffer() {
    return input_buffer;
}

EMSCRIPTEN_KEEPALIVE
const char* process_line(const char* input_line_cstr) {
    std::string input_line(input_line_cstr);

    // Redirect cout and cerr to our stringstream
    auto* old_cout = std::cout.rdbuf(output_buffer.rdbuf());
    auto* old_cerr = std::cerr.rdbuf(output_buffer.rdbuf());

    // Clear buffer from previous command
    output_buffer.str("");
    output_buffer.clear();

    try {
        if (input_line.empty()) {
            // Special case for initial call to get the banner
            std::cout << "Altair Ego: Emulating Altair BASIC 32K Rev. 3.2" << std::endl;
            std::cout << "OK" << std::endl;
        } else {
            interpreter.processLine(input_line);
        }
    } catch (const std::exception& e) {
        // processLine can throw for syntax errors etc.
        std::cout << e.what() << std::endl;
    }

    // Restore original cout and cerr
    std::cout.rdbuf(old_cout);
    std::cerr.rdbuf(old_cerr);

    // Store result and return C-string
    result_string = output_buffer.str();
    return result_string.c_str();
}

} // extern "C"

int main(int argc, char* argv[]) {
  // The main loop is now driven by JavaScript calling process_line.
  // This main function is just the entry point.
  return 0;
}

#else

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
#endif
