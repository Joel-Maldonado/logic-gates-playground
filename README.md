# logicsim-playground

A little digital logic simulator I wrote while taking an ECE class. The goal is that you can drop
inputs, outputs, and gates on a canvas, wire them together, and watch the
signals propagate. Truth tables and waveforms show up on the side so you can
poke at things and see what happens.

NOTE: This is not a serious tool, just an educational project for me and somewhere to play around with.

## What's in it

- AND / OR / NOT / XOR gates, plus a clock source and toggleable inputs
- A canvas you can pan and zoom
- Auto-generated truth table for whatever you've built
- A waveform panel that updates while the sim runs
- An EDIT mode for building, and a COMPARE mode for checking a circuit against
  a target
- Loading `.circ` files from the command line, with live reload on save

Written in C99 using [raylib](https://www.raylib.com/).
## Building

```
make        # build
make test   # run the tests
./bin/logicsim
```

You can pass a circuit file to open on launch:

```
./bin/logicsim examples/mycircuit.circ
# or
./bin/logicsim --load examples/mycircuit.circ
```

While the file is open the sim watches it and reloads on change.

## Shortcuts

Press `/` in the app for the full list. The useful ones:

| key | what it does |
| --- | --- |
| `V` | select tool |
| `1`–`7` | place input / output / AND / OR / NOT / XOR / clock |
| `Space` | run / stop |
| `.` | step one tick |
| `R` | reset sim |
| `B` / `C` | switch to Edit / Compare mode |
| `Tab` | cycle through nodes |
| `Del` | delete selected node or wire |
| drag empty canvas | pan |
| `+` / `-` | zoom |

## Layout

- `src/` — the app. `main.c` is the event loop; `logic.c` is the simulator;
  `ui_*` handles drawing and hit-testing; `app.c` holds the editor state
- `include/` and `lib/` — vendored raylib headers + static lib
- `tests/` — a tiny test harness for the logic core

