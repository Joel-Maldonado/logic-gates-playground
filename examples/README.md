# Examples

- `and_gate.circ` is the smallest useful example: two inputs, one AND gate, one output.
- `half_adder.circ` is a step up without getting too busy: it produces a `SUM` bit with XOR and a `CARRY` bit with AND.

Try them with:

```sh
./bin/logicsim examples/and_gate.circ
./bin/logicsim examples/half_adder.circ
```

Positions in these `.circ` files are optional. On load the app now auto-lays out the graph onto the editor grid.
