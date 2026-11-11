Yes. The first useful form of “intuition” is not a human-like hunch. It is a small, deterministic attention mechanism:

> Given the current board and the complete legal move list, identify facts that deserve earlier examination—but do not choose a move.

It should sit here:

```text
GENERATE → INTUIT → ANTICIPATE → EVALUATE → SELECT → DECISION
```

`INTUIT` receives only validated facts:

```text
board + bounded game memory + complete legal move list
    → IntuitionFrame
```

Its output is a traceable record, not an action:

```text
IntuitionFrame:
  candidate move
  factual tags
  projection priority
  reason codes
```

The initial version should be deliberately modest. For every legal move, attach facts such as:

- it is a capture;
- it promotes a man;
- it is the only legal move;
- it begins or continues a forced jump;
- it changes material;
- it has a known tactical consequence after one bounded projection.

The crucial rules are:

1. `INTUIT` cannot invent, remove, or declare moves legal.
2. It cannot publish a move or bypass `DECISION`.
3. It is deterministic and bounded.
4. Every cue is retained in the trace with its reason.
5. `ANTICIPATE` still constructs the actual future board; intuition only tells it what to inspect first.

The distinction becomes clean:

| Stage | Question |
|---|---|
| `GENERATE` | What moves are legal? |
| `INTUIT` | What facts make some legal moves worth looking at first? |
| `ANTICIPATE` | What actually follows from each candidate line? |
| `EVALUATE` | How good are those results under the strategy? |
| `DECISION` | Which complete, legal result is published? |

The first implementation should be called something like `I0: Tactical Geometry`. It uses only rule-visible facts and changes projection order, not the chosen move. That gives us a safe baseline: we can measure whether the cue layer reduces work or improves trace quality while preserving identical final decisions.

Only after that works should we attempt `I1`, where bounded anticipation feeds back a compact threat signal—such as “this move permits an immediate forced capture”—into the next turn’s intuition record.