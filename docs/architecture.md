# Architecture Overview

## Runtime Layers

- `app/`: Application lifecycle (`InitWindow`, main loop, resize handling).
- `simulation/`: Circuit graph ownership (`CircuitSimulator`) and convergence-based stepping.
- `core/`: Gate, pin, and wire domain model (logic/state propagation, connection lifecycle).
- `ui/`: Rendering and interaction orchestration (`UIManager`, `InputHandler`, renderers, palette).

## Simulation Model

- `LogicGate` is the base type for all gate entities and includes:
  - identity and geometry,
  - dirty-state evaluation (`markDirty`, `update`),
  - pin and associated wire ownership links,
  - `GateKind` for renderer dispatch without RTTI.
- `GatePin` manages input/output pin state and dependency links.
- `Wire` connects one output pin to one input pin and mirrors signal state for visuals.

## Simulation Step

`CircuitSimulator::update()` runs convergence passes with a hard guard (`MAX_SIMULATION_PASSES = 64`):

1. Update all wires (track state transitions).
2. Evaluate all dirty gates.
3. Repeat until no wire changes and no dirty gates remain, or guard is hit.

Returned stats:

- `passes`: number of passes consumed,
- `stable`: convergence reached,
- `oscillating`: pass guard hit (possible oscillation).

## Interaction System

`InputHandler` runs an explicit interaction mode machine:

- `Idle`, `PaletteDrag`, `WireDraw`, `GatePressPending`, `GateDrag`, `WirePointDrag`, `Pan`.

Key behavior:

- Top-most hit testing uses reverse iteration order to match draw stacking.
- Selecting a gate/wire brings it to front for intuitive layering.
- Press-capture toggles input sources only on click release over the same source and under drag threshold.
- `Shift` axis-locks drag; `Alt` temporarily disables snapping.

## Rendering

- `GateRenderer` renders by `GateKind` (no `dynamic_cast`).
- `WireRenderer` delegates wire drawing to `Wire::draw` and preview pathing to `WireRouter`.
- `UIManager` renders debug overlay (toggle with `F1`) including simulation stats and interaction mode.

## Testing

`tests/tests.cpp` provides deterministic unit coverage for:

- gate truth tables,
- deep propagation convergence,
- fanout behavior,
- insertion-order invariance,
- oscillation detection,
- gate deletion wire cleanup,
- interaction helper threshold/axis-lock behavior.
