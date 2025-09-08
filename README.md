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

## Building

AltairEgo uses GNU Autotools for building:

```bash
./autogen.sh
./configure
make
sudo make install
```

## Usage

### Interactive Mode
```bash
altair_ego
```

### File Mode
```bash
altair_ego program.bas
```
### WebApp Mode

[mastermind.bas](https://jlowder.github.io/AltairEgo/spa/mastermind.html)

## Example Programs

The `working-examples/` directory contains several classic BASIC games that demonstrate the interpreter's capabilities:

- **mastermind.bas** - Code-breaking logic game
- **amazing.bas** - Maze generation program
- **awari.bas** - Ancient African strategy game
- **depth_charge.bas** - Naval warfare simulation
- **calendar.bas** - Calendar display utility
- **3dplot.bas** - 3D plotting demonstration

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
