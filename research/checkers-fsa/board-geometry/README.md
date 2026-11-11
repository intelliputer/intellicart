# Checkers board geometry research module

This module is intentionally outside `checkers-core`. It verifies a proposed
32-square Checkers representation before the representation becomes part of
the portable FSA.

## Numbering

Squares are numbered left-to-right, top-to-bottom on the playable squares of
an 8×8 television board. Row `0` is the top of the display.

```text
row 0:    0     1     2     3
row 1:  4     5     6     7
row 2:    8     9    10    11
row 3: 12    13    14    15
row 4:   16    17    18    19
row 5: 20    21    22    23
row 6:   24    25    26    27
row 7: 28    29    30    31
```

The human begins at the bottom (squares `20..31`) and moves toward decreasing
row numbers. The agent begins at the top (`0..11`) and moves toward increasing
row numbers. This module does not implement legal moves; it only establishes
coordinate and position invariants.

## Build

```sh
cmake -S research/checkers-fsa/board-geometry -B /tmp/board-geometry-build
cmake --build /tmp/board-geometry-build
ctest --test-dir /tmp/board-geometry-build --output-on-failure
```

Promotion into `checkers-core` happens only after these fixtures and the
chosen CP1610 display mapping agree.
