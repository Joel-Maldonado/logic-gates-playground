# Logic Gates Playground

A simple, interactive playground for experimenting with digital logic gates and circuits. This project provides a visual environment where you can create, connect, and simulate basic logic circuits, primarily built as a learning experience.

![Logic Gates Playground](docs/screenshot.png)

## Overview

- Create various logic gates (AND, OR, XOR, NOT)
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
git clone https://github.com/rnoc/logic-gates-playground.git
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
git clone https://github.com/rnoc/logic-gates-playground.git
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

## Controls

- Left click: select components and wires
- Drag from palette item to canvas: place a new component
- Left drag on gate body: move component
- Left click + drag wire points: adjust wire route
- Left click on input source body: toggle input state
- Left click on output pin then input pin: create wire
- Right click or `Esc`: cancel current transient action / clear selection
- Middle drag or right drag: pan camera
- Mouse wheel: zoom
- `Backspace`/`Delete`: delete selected gate or wire
- `G`: toggle grid snapping
- Hold `Shift` while dragging: axis lock drag
- Hold `Alt` while dragging: temporarily disable snapping
- `F1`: toggle debug overlay

## Smoke Test Checklist

Detailed smoke script: `docs/smoke-test.md`.

1. Build and run the app.
2. Drag each palette component onto the canvas.
3. Wire a multi-stage circuit (8+ gates) and verify output updates in one update cycle.
4. Create overlapping gates and verify top-most gate gets selected.
5. Toggle an input with click (single toggle) and drag it (no toggle).
6. Pan/zoom and resize the window; verify interaction remains stable.
7. Create a NOT self-loop and verify debug overlay reports oscillation.

## Known Constraints

- Oscillating feedback circuits are detected and clamped after a maximum number of simulation passes.
- There is currently no circuit persistence (save/load) feature.
