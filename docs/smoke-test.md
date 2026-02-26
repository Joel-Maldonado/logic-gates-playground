# Smoke Test Script

## Build

```bash
cmake -S . -B build
cmake --build build -j4
ctest --test-dir build --output-on-failure
```

## Manual Runtime Checks

1. Run `./build/bin/logic-gates-playground`.
2. Drag each palette item onto the canvas.
3. Wire an 8-stage circuit and verify the output updates immediately.
4. Overlap two gates and verify the visible top gate is selected on click.
5. Click an input source body once (single toggle), then drag it (no toggle).
6. Hold `Shift` while dragging (axis lock) and `Alt` while dragging (snap disabled).
7. Use right-click and `Esc` during wire draw and palette drag to verify clean cancel.
8. Press `F1` and verify debug overlay shows pass count/stability/mode.
