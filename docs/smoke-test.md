# Smoke Test Script

## Build

```bash
cmake -S . -B build
cmake --build build -j4
ctest --test-dir build --output-on-failure
```

## Manual Runtime Checks

1. Run `./build/bin/logic-gates-playground`.
2. Verify panel shell renders: left component library, top toolbar, right inspector, bottom status bar.
3. Drag each palette item onto the canvas and verify creation works.
4. While dragging from palette, verify a full gate ghost (shape + pins) appears on canvas before drop.
5. Toggle snap from toolbar/command palette and verify ghost/drop coordinates match with snap on/off; hold `Alt` to temporarily bypass snap.
6. Verify OR and XOR are visually distinct in both palette icons and canvas symbols.
7. Verify wire endpoints meet gate boundaries (no floating connector gap) for OR/XOR/NOT.
8. Build an 8-stage circuit and verify propagation updates the output in one update cycle.
9. Overlap gates and verify top-most shape receives selection (shape-aware hit test).
10. Click an input source once (single toggle) and drag it (no toggle).
11. Start wire draw from an output pin and complete only on a valid free input pin.
12. Select wire elbows and drag points; verify orthogonal routing remains intact.
13. Use `Shift+Drag` marquee to select multiple gates/wires.
14. Use `Shift+Click` to add/remove a gate or wire from selection.
15. Use arrow keys to nudge selected gates; hold `Shift` for larger nudge.
16. Use `Ctrl/Cmd+Z` and `Ctrl/Cmd+Shift+Z` to undo/redo gate moves and additions.
17. Use `Ctrl/Cmd+D` to duplicate selected gates and internal wires.
18. Use `Ctrl/Cmd+A` to select all, then `F` to frame selection.
19. Open command palette with `Ctrl/Cmd+K`, run a command by number, and verify action executes.
20. Use right-click and `Esc` during palette drag, wire draw, and marquee to verify clean cancel behavior.
21. Press `G` to toggle grid visibility, open command palette and run `Toggle Grid Snap`, and press `F1` to verify debug overlay stats.
