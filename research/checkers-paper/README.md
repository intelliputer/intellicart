# Checkers FSA paper

This is the manuscript workspace for a future arXiv preprint describing the
portable, deterministic finite-state Checkers agent being developed in this
repository.

It intentionally does not claim novelty or game-playing strength yet. Those
claims need literature review, historical comparison, complete rules, policy
experiments, and reproducible measurements.

## Build

With a TeX distribution installed, run pdflatex on main.tex twice. The
manuscript is self-contained at this stage. A bibliography and archival
references will be added only after primary sources have been checked.

## Evidence available today

- ../checkers-evidence/FSA-LOGIC.md — formal state, input alphabet, and
  output alphabet.
- ../checkers-evidence/FSA-LOGIC.png — readable rendered state-machine proof.
- ../checkers-fsa/ — portable C++11 core, fixtures, and implementation plan.
- ../checkers-ai/PERIOD-COMPUTER-PLAYERS.md — initial historical research lead
  list; it is not yet a formal related-work survey.

## Before submission

1. Complete the Checkers rules and FSA milestones.
2. Establish baseline players and a reproducible evaluation protocol.
3. Review related work from primary or archival sources.
4. Add affiliation, repository revision, and final publication date.
5. Obtain a careful technical and factual review.
