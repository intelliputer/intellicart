# Checkers FSA logic

This is the target control FSA for the portable Checkers agent. Policy ranks
legal projected moves in the evaluation and selection states; it is not the
state machine itself.

```mermaid
stateDiagram-v2
    [*] --> q0

    state "q0: WAIT_REQUEST" as q0
    state "q1: SNAPSHOT_REQUEST" as q1
    state "q2: RECONCILE_RESULT" as q2
    state "q3: GENERATE" as q3
    state "qI: INTUIT" as qI
    state "q4: ANTICIPATE" as q4
    state "q5: EVALUATE" as q5
    state "q6: SELECT" as q6
    state "q7: DECISION" as q7
    state "qE: ERROR" as qE
    state "q8: WAIT_RESULT" as q8
    state "IntuitionMemory\nframe + history + fixed-range bias" as mI

    q0 --> q1: committed request(seq)
    q1 --> qE: invalid protocol / stale seq
    q1 --> qE: invalid board
    q1 --> q2: valid snapshot

    q2 --> qE: prior result contradicts expected board
    q2 --> q3: result reconciled; update intuition history
    q2 -. observed reply; saturating bias update .-> mI

    q3 --> qE: no legal moves / terminal state
    q3 --> q3: more pieces to scan
    q3 --> qI: complete legal move list

    qI --> qI: tag next legal candidate
    qI --> q4: IntuitionFrame complete
    q4 --> q4: project next candidate/line
    q4 --> q5: all projections complete

    q5 --> q5: evaluate next projection
    q5 --> q6: all candidates scored

    q6 --> q7: deterministic best move selected
    q7 --> q8: write move + diagnostic; READY last
    q7 -. store pending decision record .-> mI

    qE --> q8: write error; ERROR last
    q8 --> q0: response acknowledged; retain pending decision
    mI -. factual cues + historical bias .-> qI
```

## Initial state

```text
q0 = WAIT_REQUEST
```

## Input alphabet, Σ

The input alphabet is the set of normalized events the FSA accepts from its
mailbox adapter, rules engine, memory, and bounded projection work.

```text
Σ = {
  committed_request(seq),
  invalid_protocol(error),
  stale_sequence,
  valid_board,
  invalid_board,
  prior_result_matches,
  prior_result_mismatch,
  more_generation_work,
  generation_complete,
  no_legal_move,
  more_intuition_work,
  intuition_complete,
  more_projection_work,
  projections_complete,
  more_evaluation_work,
  evaluations_complete,
  response_acknowledged,
  new_sequence
}
```

## Output alphabet, Γ

The output alphabet is intentionally separate from `Σ`. It is the observable
mailbox/trace result produced by the controller.

```text
Γ = {
  BUSY(seq),
  READY(seq, move, diagnostic),
  ERROR(seq, error_code),
  trace(phase, board_hash, candidate, cues, bias, score)
}
```

## Intuition memory and contract

`qI = INTUIT` is a bounded, deterministic attention state between complete
legal-move generation and projection. It uses explicit `IntuitionMemory`, not
a transient hint. The controller plus this fixed-size memory is an extended
FSA: the controller states are finite, and the complete configuration remains
finite because every record has a fixed width and capacity.

`IntuitionMemory` has two bounded parts:

- `IntuitionFrame`, retained from qI until the request is acknowledged:
  source-board key, candidate count and cursor, cue mask, priority, and reason
  code for every legal candidate.
- `IntuitionHistory`, retained across turns: the previous decision trace, the
  observed opponent-reply class, fixed counters for cue/outcome pairs, and
  fixed-range cue-bias values.

The first version, `I0: Tactical Geometry`, may tag facts such as capture,
promotion, only-legal-move, forced-jump status, material change, and a
bounded projected tactical consequence. It may initially leave
`IntuitionHistory` read-only, but its record format is present from the start
so later versions can compare a prior expectation with the observed reply.

The historical update follows the FSA loop. The DECISION state stores a bounded
pending-decision record. The record survives the q8 to q0 transition; after
the next board arrives, RECONCILE_RESULT compares it with the recorded decision
and updates cue-bias values by fixed, saturating increments. INTUIT then uses
those biases only to order its next IntuitionFrame. This is iterative
remembered experience, not an unbounded learner.

- It cannot create, remove, or make a move legal.
- It cannot publish a move or bypass `q7 = DECISION`.
- It must preserve every legal candidate for later projection and evaluation.
- It may order projection, but not suppress candidates.
- Historical bias may adjust priority, but not legality, evaluation, selection,
  or mailbox publication.
- Its work, storage, and trace record are bounded and deterministic.

`ANTICIPATE` constructs candidate futures; `INTUIT` uses present cues and
bounded remembered outcomes to decide which legal candidates deserve earlier
examination and why.

## Current implementation boundary

The portable core currently implements the `q0 → q1 → q2 → q7/qE → q8`
request/response skeleton. The rules module supplies the legal-move evidence
needed to implement `q3`. `qI` and `IntuitionMemory` are specified but not
implemented. Projection, evaluation, and selection remain the next FSA stages;
they will be introduced only after the complete rules set is fixture-tested.
