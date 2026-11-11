# Checkers FSA implementation plan

## Objective

Deliver a deterministic Checkers computer opponent that can be tested natively,
connected to a Mira-hosted CP1610 surface, and then served by the Teensy
through the Intellicart mapped-RAM mailbox.

The agent remains a stateful finite-state controller with explicit memory,
anticipation, and policy. It is not an LLM or a black-box model.

## Completed groundwork

- Portable C++11 `checkers-core`, independent of Arduino, Mira, and SDL.
- Versioned 32-word mailbox ABI and desktop mailbox harness.
- FSA request/response sequencing, BUSY/READY ordering, and protocol fixtures.
- Separate board-geometry research module proving the 32-square mapping.
- Core board validation for mailbox `BOARD` requests.
- First rules module: uncrowned-men quiet moves, single captures,
  compulsory-capture filtering, canonical move order, and legal move
  application.

## Remaining delivery milestones

| # | Milestone | Scope | Acceptance evidence |
| --- | --- | --- | --- |
| 1 | Promotion and kings | Crown a man at the far row; add king quiet moves and single captures in both directions. | Native fixtures prove crowning, king movement, king captures, and mixed man/king positions. |
| 2 | Multi-jumps and continuation | Generate full capture paths and preserve the forced moving piece until the turn ends. | Fixtures cover branching paths, longest paths, continuation state, and crowning at the end of a capture turn. |
| 3 | First playable FSA | Replace `BOARD → UNIMPLEMENTED` with `GENERATE → PROJECT → SELECT → READY`; choose a stable lowest legal move only. | Every fixture board yields one expected mailbox response and projected result board. |
| 4 | Game history | Add bounded move/repetition and no-progress history; identify draw/terminal outcomes. | Repetition and terminal-position fixture traces are reproducible. |
| 5 | Desktop + Mira transport | Implement the optional Mira mailbox peripheral around the same portable core. | CP1610-side request/response trace agrees with native fixture trace. |
| 6 | CP1610 Checkers surface | Build the display, controller, board encoding, and mailbox client with SDK1600. | Mira renders fixtures, accepts a human move, and displays the computer response. |
| 7 | Teensy deployment | Add the thin safe-point adapter to Intellicart firmware and prepare the dedicated test SD card. | Real hardware completes the same trace without timing failures, stale replies, or display loss. |

## Strength track — after the playable baseline

Strength is intentionally separate from correctness and deployment.

1. Add policy terms one at a time: capture length, promotion, safety, centre,
   and material.
2. Add one bounded opponent reply to each projection.
3. Add deeper deterministic search only if the measured Teensy service budget
   permits it.

Each policy change needs a position where that term is the sole reason the
selected move changes. No strength feature may alter legal-move generation or
mailbox correctness.

## Current next task

Milestone 1: promotion and kings. Multi-jumps remain explicitly unsupported
until Milestone 2, so the core never silently produces a partial or illegal
turn.
