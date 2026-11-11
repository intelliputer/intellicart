# Checkers FSA — next-session handoff

## Resume point — 11 November 2026

The last committed research gate is `a568f42` (`Define Checkers research
publication gate`). The working tree now contains an uncommitted refinement of
the FSA evidence, paper, bibliography, rendered PDF, and reference library.
Review it as one coherent documentation change before committing. Do not add
unrelated `NOTES.md` or user-authored `research/ProjectNotes.txt` changes
without reviewing them separately.

The project has a credible research direction, but no experimental result yet:

> **Bounded intuition is an auditable FSA stage that uses factual legal-move
> cues and fixed-size remembered outcomes to order projection work. It never
> changes legality, removes a candidate, selects a move, or publishes a
> response.**

Checkers remains the test instrument, not the claimed contribution.

## Current FSA contract

The evidence diagram and paper now use this state vocabulary:

    q0 WAIT_REQUEST
    q1 SNAPSHOT_REQUEST
    q2 RECONCILE_RESULT
    q3 GENERATE
    qI INTUIT
    q4 ANTICIPATE
    q5 EVALUATE
    q6 SELECT
    q7 DECISION
    qE ERROR
    q8 WAIT_RESULT

`IntuitionMemory` is explicit in the diagram:

- `q7` stores a bounded pending-decision record.
- That record survives `q8 -> q0`.
- `q2` compares the observed reply with the record and applies fixed,
  saturating updates to fixed-range cue biases.
- `qI` combines the current factual cues with those historical biases to make
  an `IntuitionFrame`, which orders the complete legal candidate list.

The purple dashed edges in `research/checkers-evidence/FSA-LOGIC.png` are the
memory/bias interactions. The store is deliberately outside the control path:
it is not an error state and is not a mailbox transition.

### I0 and I1 scope

- **I0: Tactical Geometry** is the first implementation. It records only
  rule-visible facts: capture, promotion, only legal move, forced jump,
  material change, and a bounded tactical consequence. Its priority order may
  be tested against canonical order while preserving the same final candidate
  set.
- **I1** comes only after I0 evidence exists. A compact, bounded result from
  anticipation (for example, "permits an immediate forced capture") may be
  recorded for the next turn. It remains a priority signal, not policy
  replacement or training.

## What is implemented and tested

- `research/checkers-fsa/checkers-core` is portable ISO C++11.
- The 32-word mailbox ABI, request/response sequencing, malformed-board
  handling, and atomic `READY`/`ERROR` publication are implemented.
- `board-geometry` independently validates the 32-square mapping, coordinate
  round trips, edges, opening position, and invalid encodings.
- The rules engine supports uncrowned-men quiet moves and single captures,
  compulsory capture, canonical ordering, and legal men-move application.
- The intentional current boundaries are `PROMOTION_NOT_IMPLEMENTED` and
  `MULTI_JUMP_NOT_IMPLEMENTED`.
- `qI`, historical bias, anticipation, scoring, selection, and an end-to-end
  computer-player `BOARD` response are specified, not implemented.

Run this before and after any rules increment:

    cmake -S research/checkers-fsa/checkers-core -B /tmp/checkers-core-build
    cmake --build /tmp/checkers-core-build
    ctest --test-dir /tmp/checkers-core-build --output-on-failure

## Immediate implementation task

Finish the rules before adding I0. The next narrow increment is promotion and
kings:

1. Add fixtures for crowning on the far row.
2. Generate and apply king quiet moves in each valid diagonal direction.
3. Generate and apply single king captures in each valid diagonal direction.
4. Confirm captures still suppress quiet moves and canonical ordering remains
   stable.
5. Preserve the explicit multi-jump boundary; do not implement continuation in
   this increment.

Only after complete rules fixtures should the project connect `GENERATE` to
`INTUIT`, first with I0 operating as a transparent ordering layer. Do not add
an LLM, neural model, replay buffer, or reinforcement-learning policy.

## Paper position and evidence gate

The current draft is:

- `research/checkers-paper/main.tex`
- `research/checkers-paper/checkers-fsa-paper.pdf`

Its title is *Auditable Bounded Intuition on Constrained Game Hardware: A
Checkers Case Study*. The literature section now groups related work as:

1. automata and controller synthesis;
2. finite machines as game strategies;
3. direct artificial-intuition work;
4. finite-memory policy improvement; and
5. embedded continual-learning hardware/software.

The paper's differentiator is **not** "machines can have human intuition."
It is the engineering contract that makes remembered prioritization bounded,
deterministic, traceable, and unable to bypass rules or publication controls.

Do not enlarge the paper again until implementation yields evidence for all
of these:

1. Complete rules and an operational `INTUIT`/`ANTICIPATE`/`DECISION` contract.
2. Fixtures proving legal, deterministic, atomically published moves and
   bounded trace records.
3. Measured time per state and per decision, code size, RAM, and retained
   history/projection limits.
4. Matching fixtures and traces on desktop, Mira, and the Intellicart/Teensy
   path.
5. A comparison with a simple conventional loop on defect localization,
   reproducibility, memory, and timing. Playing strength is secondary.

If those results do not materialize, retain this as a technical report rather
than an arXiv submission.

## Reference library

`references/README.md` is the themed local index and
`research/checkers-paper/references.bib` is the paper citation index.

    references/automata-foundations/
    references/controller-synthesis/
    references/game-strategy-and-testing/
    references/artificial-intuition/
    references/finite-state-memory/
    references/embedded-adaptation/

The direct artificial-intuition lineage is Johnny--Trovati--Ray (2020),
Trovati et al. (2022), and Trovati--Cullen (2023). The 2023 author manuscript
is local. The 2022 full text remains cited but was unavailable as a valid
command-line download; do not replace it with a non-PDF landing page.

## Deliberate exclusions for now

- No policy scoring, search, or `INTUIT` implementation before complete rules.
- No multi-jump, draw/repetition rules, Mira adapter, CP1610 surface, Teensy
  firmware, or SD-card work during the promotion-and-kings increment.
- No novelty claim, benchmark conclusion, or hardware-performance claim before
  measured evidence.

## Completion condition for the next coding session

Promotion and kings have isolated passing fixtures; all existing men-only
fixtures remain green; multi-jumps are still explicitly unsupported; and
`research/ProjectNotes.txt` records the completed scope and the next narrow
increment.
