# Logic Gates Playground

A simple, interactive playground for experimenting with digital logic gates and circuits. This project provides a visual environment where you can create, connect, and simulate basic logic circuits, primarily built as a learning experience.

![Logic Gates Playground](docs/screenshot.png)

## Overview

- Create various logic gates (AND, OR, XOR, NOT, NAND, NOR, XNOR)
- Add input sources and output sinks
- Connect gates with wires to build circuits
- Drag and drop components for easy circuit design


## Requirements

- C++17 compatible compiler
- CMake 3.14 or higher
- [raylib](https://www.raylib.com/) (automatically fetched by CMake if not found)

## Building from Source

### On Linux/macOS

```bash
# Clone the repository
git clone https://github.com/username/logic-gates-playground.git
cd logic-gates-playground

# Build using the provided script
chmod +x build.sh
./build.sh

# Run the application
./build/bin/logic-gates-playground
```

### On Windows

```bash
# Clone the repository
git clone https://github.com/username/logic-gates-playground.git
cd logic-gates-playground

# Build using the provided script
build.bat

# Run the application
build\bin\logic-gates-playground.exe
```

### Manual Build

```bash
mkdir -p build
cd build
cmake ..
cmake --build . --config Release
```

The executable will be located in the `build/bin` directory.
