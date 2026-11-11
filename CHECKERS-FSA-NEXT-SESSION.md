# Checkers FSA — next-session handoff

## Resume point

The portable Checkers work is on a clean, committed foundation as of
4ff69ce, Refine Checkers FSA paper.

The next implementation increment is promotion and kings in the rules module:

- Crown a man that ends a legal move on its far row.
- Generate and apply king quiet moves and single captures in both diagonal
  directions.
- Keep the explicit multi-jump-not-implemented boundary.
- Do not add policy, search, or platform adapters in this increment.

## What already works

- board-geometry independently validates the 32-square numbering, coordinate
  mapping, edge behavior, opening position, and invalid encodings.
- checkers-core accepts and validates mailbox requests, including sequence and
  malformed-board errors.
- The rules module supports uncrowned-men quiet moves and single captures,
  compulsory captures, canonical ordering, and applying legal men moves.
- Current intentional rules boundaries are PROMOTION_NOT_IMPLEMENTED and
  MULTI_JUMP_NOT_IMPLEMENTED.

The FSA vocabulary is:

    q0 WAIT_REQUEST
    q1 SNAPSHOT_REQUEST
    q2 RECONCILE_RESULT
    q3 GENERATE
    q4 ANTICIPATE       (performs bounded projection)
    q5 EVALUATE
    q6 SELECT
    q7 DECISION         (outgoing transition writes READY)
    qE ERROR            (outgoing transition writes ERROR)
    q8 WAIT_RESULT

ANTICIPATE is the state name; projection remains a valid operation and
transition condition. DECISION and ERROR are conditions, not names for
mailbox-write operations.

## Paper novelty in one page

Do not claim that this work invented finite-state game strategies or solved
Checkers. Those are established subjects. The paper's possible contribution is
an end-to-end method for a small deterministic game player that carries an
explicit FSA from desktop tests to constrained hardware.

The argument rests on four connected features:

- ANTICIPATE makes bounded projection visible rather than hiding it inside a
  search loop.
- DECISION separates choosing a move from publishing a complete mailbox
  response.
- One portable core is intended to serve desktop tests, Mira, the CP1610 game
  surface, and the Teensy-assisted cartridge system.
- The source keeps an evidence trail: FSA diagram, protocol and rule fixtures,
  projected traces, resource measurements, and hardware observations.

The novelty is provisional. It must be supported by complete transition and
error coverage, cross-platform fixture replay, measured time and memory bounds
for ANTICIPATE, and comparisons with declared deterministic baselines. Better
Checkers play is a separate empirical question, not the paper's central claim.

## Paper publication gate

Checkers is the case study, not the paper's contribution. Do not spend further
time polishing or enlarging the manuscript until implementation produces
evidence for all of these:

1. An operational contract for ANTICIPATE, DECISION, and mailbox publication.
2. Fixtures showing that every published move is legal, deterministic, tied to
   a bounded projected line, and never partly published.
3. Measured time per FSA step and complete decision, code size, working memory,
   and retained projection-data limit.
4. The same fixtures and replay traces on desktop, Mira, and the intended
   hardware path.
5. A comparison with a simple conventional game loop: defect localization,
   reproducibility, memory, and timing. Playing strength is secondary.

If those conditions are not met, retain the document as a technical report.
Do not present or submit a paper whose result is only another Checkers player.

## First commands

From the repository root:

    cmake -S research/checkers-fsa/checkers-core -B /tmp/checkers-core-build
    cmake --build /tmp/checkers-core-build
    ctest --test-dir /tmp/checkers-core-build --output-on-failure

All fixtures must pass before and after the next increment.

## How to implement the next increment

1. Read checkers-core/include/checkers/rules.hpp,
   checkers-core/src/rules.cpp, and tests/fixture_runner.cpp.
2. Add isolated fixtures before or alongside the implementation:
   - a man reaches its far row and becomes a king;
   - a king makes quiet moves in all valid diagonal directions;
   - a king makes a single capture in all valid diagonal directions;
   - king capture suppresses quiet moves;
   - applyMove preserves or correctly removes king bits;
   - a position that needs another jump still reports
     MULTI_JUMP_NOT_IMPLEMENTED.
3. Extend generation and application only enough to satisfy those fixtures.
4. Preserve canonical move ordering and validate moves through generation.
5. Re-run the fixture suite and update research/ProjectNotes.txt with the
   completed scope and the next narrow increment.

## Do not do yet

- Do not introduce a policy, scoring rule, search, or LLM component.
- Do not connect move generation to the FSA response path yet.
- Do not modify Mira, CP1610 code, Teensy firmware, or SD-card assets.
- Do not implement multi-jump continuation or draw/repetition handling in the
  promotion-and-kings increment.

## Reference material

- Active roadmap: research/checkers-fsa/IMPLEMENTATION-PLAN.md and
  research/checkers-fsa/NEXT-STEPS.md.
- FSA evidence: research/checkers-evidence/FSA-LOGIC.md and FSA-LOGIC.png.
- Paper: research/checkers-paper/main.tex and checkers-fsa-paper.pdf.
- Formal foundation: references/Finite_Automata_Behavior_and_Synthesis.pdf by
  B. A. Trakhtenbrot and Ya. M. Barzdin.

## Completion condition

Promotion and kings have isolated, passing fixtures; existing men-only
fixtures remain green; multi-jumps remain explicitly unsupported; and the
documentation states the resulting boundary.
