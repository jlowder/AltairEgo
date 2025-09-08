# AltairEgo

A 1970s-era BASIC language interpreter designed to run classic programs from David Ahl's "101 Basic Computer Games" collection.

## Overview

AltairEgo is a faithful implementation of vintage BASIC, capturing the essence of early personal computing. The interpreter supports the core BASIC language features needed to run educational and entertainment programs from the golden age of home computing.

## Features

- Line-numbered BASIC program execution
- Interactive and file-based program loading
- Classic BASIC statements: PRINT, INPUT, IF/THEN, FOR/NEXT, GOTO, GOSUB/RETURN
- Mathematical functions and expressions
- String and numeric variable support
- Compatible with programs from "101 Basic Computer Games"
- Web application support via Emscripten

## Building

AltairEgo uses GNU Autotools for building the command-line interpreter:

```bash
./autogen.sh
./configure
make
sudo make install
```

### Building for the Web (Emscripten)

To compile the interpreter as a web application, you'll need the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html). 

The `working-examples/spaify` directory contains scripts to facilitate building Single Page Applications (SPAs) from BASIC source files:

- **`spaify`**: This script takes a `.bas` file and embeds it into a standalone HTML file. It offers both a retro-themed template and a simple one.

  ```bash
  ./working-examples/spaify/spaify working-examples/amazing.bas
  ```

- **`loaderify`**: This script builds `spa/loader.html`, a generic loader page that can run any of the example programs.

  ```bash
  ./working-examples/spaify/loaderify
  ```

For more direct control, you can use the `em++` command. For example:

```bash
em++ -I src src/*.cpp -o altair_ego.html -s "EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap', 'stringToUTF8']" -s "ASYNCIFY_IMPORTS=['await_input_from_js']" --shell-file shell.html -s SINGLE_FILE=1 -s ASYNCIFY -fexceptions
```

Additionally, the `spa/opt` directory contains a `README` with instructions on how to optimize the generated WebAssembly using `wasm-opt`.

## Usage

### Interactive Mode
```bash
altair_ego
```

### File Mode
```bash
altair_ego program.bas
```
## Example Programs

The `working-examples/` directory contains several classic BASIC games that demonstrate the interpreter's capabilities. You can run them from the command line or play them directly in your browser.

- **3dplot.bas** - 3D plotting demonstration.
  - [Source](working-examples/3dplot.bas)
  - <a href="https://jlowder.github.io/AltairEgo/spa/3dplot.html" target="_blank">Play in browser</a>

- **amazing.bas** - Maze generation program.
  - [Source](working-examples/amazing.bas)
  - <a href="https://jlowder.github.io/AltairEgo/spa/amazing.html" target="_blank">Play in browser</a>

- **awari.bas** - Ancient African strategy game.
  - [Source](working-examples/awari.bas)
  - <a href="https://jlowder.github.io/AltairEgo/spa/awari.html" target="_blank">Play in browser</a>

- **calendar.bas** - Calendar display utility.
  - [Source](working-examples/calendar.bas)
  - <a href="https://jlowder.github.io/AltairEgo/spa/calendar.html" target="_blank">Play in browser</a>

- **depth_charge.bas** - Naval warfare simulation.
  - [Source](working-examples/depth_charge.bas)
  - <a href="https://jlowder.github.io/AltairEgo/spa/depth_charge.html" target="_blank">Play in browser</a>

- **mastermind.bas** - Code-breaking logic game.
  - [Source](working-examples/mastermind.bas)
  - <a href="https://jlowder.github.io/AltairEgo/spa/mastermind.html" target="_blank">Play in browser</a>

### Running an Example

```bash
altair_ego working-examples/3dplot.bas
                               3D PLOT
              CREATIVE COMPUTING  MORRISTOWN, NEW JERSEY




                        *
                    *   *  *
                 *  *   *  *   *
             *   *  *   *  *   *  *
             *   *  *   *  *   *  *
             *   *  *   *  *   *  *
          *  *   *  *   *  *   *  *   *
          *  *   *   *  *   *  *  *   *
          *  *   *   *   *  *  *  *   *
      *   *  *   *    *  *   * *  *   *  *
      *   *  *    *   *    * *  * *   *  *
      *   *   *   *     *   *  **  *  *  *
      *   *   *    *      *    * * *  *  *
      *   *   *     *       *    * *  *  *
      *   *   *      *        *      **  *
      *   *    *       *         *       *
      *   *    *        *           *       *
      *   *     *         *            *        *
      *   *     *          *             *         *
      *   *     *          *               *         *
   *  *   *     *           *              *          *
      *   *     *          *               *         *
      *   *     *          *             *         *
      *   *     *         *            *        *
      *   *    *        *           *       *
      *   *    *       *         *       *
      *   *   *      *        *      **  *
      *   *   *     *       *    * *  *  *
      *   *   *    *      *    * * *  *  *
      *   *   *   *     *   *  **  *  *  *
      *   *  *    *   *    * *  * *   *  *
      *   *  *   *    *  *   * *  *   *  *
          *  *   *   *   *  *  *  *   *
          *  *   *   *  *   *  *  *   *
          *  *   *  *   *  *   *  *   *
             *   *  *   *  *   *  *
             *   *  *   *  *   *  *
             *   *  *   *  *   *  *
                 *  *   *  *   *
                    *   *  *
                        *

```

## BASIC Language Support

AltairEgo implements core BASIC features including:

- **Variables**: Numeric and string variables (A-Z, A$-Z$)
- **Control Flow**: IF/THEN, FOR/NEXT loops, GOTO, GOSUB/RETURN
- **I/O**: PRINT, INPUT statements with formatting
- **Functions**: Mathematical functions (SIN, COS, RND, etc.)
- **Arrays**: Single and multi-dimensional arrays
- **String Operations**: Basic string manipulation

## Project Structure

```
src/
├── main.cpp          # Entry point and file handling
├── interpreter.cpp   # Core BASIC interpreter logic
├── parser.cpp        # BASIC statement parsing
├── lexer.cpp         # Tokenization and lexical analysis
├── functions.cpp     # Built-in BASIC functions
└── variable.cpp      # Variable management system

spa/
├── *.html            # Standalone HTML versions of the example games
└── opt/              # Scripts and instructions for optimizing the Wasm output

working-examples/
├── *.bas             # Example BASIC programs
└── spaify/           # Scripts to build the web applications
```

## Compatibility

This interpreter aims for compatibility with BASIC programs from the 1970s era, specifically those found in educational computing books like "101 Basic Computer Games". While not a complete implementation of any specific BASIC dialect, it supports the most commonly used features from that period.

## Contributing

Contributions are welcome! Please focus on maintaining compatibility with vintage BASIC programs and preserving the authentic feel of 1970s computing.

## License

MIT

## Acknowledgments

- David Ahl for "101 Basic Computer Games"
- The early personal computing pioneers who made BASIC accessible to everyone
- The vintage computing community for preserving these classic programs
