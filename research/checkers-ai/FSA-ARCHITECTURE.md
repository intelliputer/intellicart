# Checkers agent: state, memory, anticipation, and strategy

This note refines the terminology in [PROPOSAL.md](PROPOSAL.md).  The
Checkers opponent is deterministic and purpose-built, but it is more than a
move-scoring routine.

**Policy is strategy.**  It expresses which anticipated move outcome is
preferred.  **The FSA is the stateful control mechanism.**  It retains
context, advances the interaction cycle, and makes an anticipation possible
before an action is committed.  The policy must not be mistaken for either
the controller or its memory.

## The agent's cycle

```text
CP1610 request/result                 Teensy agent
---------------------                 ---------------------------------
board + turn request  ------------->  remember request and context
                                      generate possible experiments
                                      anticipate their consequences
                                      apply strategy to rank them
response move         <-------------  enact one selected move
next board/result     ------------->  compare, remember, continue
```

For Checkers, a proposed move is an **experiment**.  The board returned by
the game surface after the move is its **result**.  The agent's interaction is
the pair: `⟨proposed move, observed board result⟩`.  A normal test surface can
make this result deterministic, but retaining the distinction is still useful:
it lets the firmware detect stale mailbox data, an invalid surface update, or
a mismatch between a predicted and observed board.

## Four separate responsibilities

| Part | It owns | It does not decide alone |
| --- | --- | --- |
| Memory | Board snapshot, side to move, forced-jump continuation, request sequence, last enacted move, repetition/history, learned or configured transition records | What to do next |
| FSA | The phase of the interaction and the legal transition to the next phase | Which legal move is best |
| Anticipation | Projected board states and predicted responses for candidate moves or bounded lines | The preference between satisfactory projections |
| Policy | Strategy: the deterministic ranking of anticipated outcomes | Mailbox validity, control flow, or state preservation |

The first version's policy can remain simple: compulsory capture, capture
length, promotion, immediate safety, centre preference, and a stable square
number tie break.  Those are strategy terms, not FSA states.

## Required Teensy state

The FSA needs explicit, durable memory.  This is not optional bookkeeping.

```text
AgentMemory
  observedBoard          latest validated 32-square position
  requestedBoard         immutable board snapshot for the current request
  priorInteraction       last intended move and observed result
  history                bounded move / repetition / capture-continuation record
  candidateSet           legal moves being considered
  projection             one projected candidate state or bounded line
  selectedMove           response held until it is committed
  requestSequence        prevents acting on a stale mailbox request
  phase                  the FSA's current state
```

`requestedBoard` must not be changed while a turn is being evaluated.  Each
candidate is applied to a copy of that state; the selected move is then
published atomically to the mailbox.  On the following request, the surface's
board is compared with the expected consequence before the new turn begins.

## FSA: control, not strategy

The small mailbox implementation should be an incremental FSA.  Its states
are intentionally concerned with the interaction cycle and bus safety, not
with whether a central square is valuable.

```text
WAIT_REQUEST
  -> SNAPSHOT_REQUEST       command and sequence are complete
  -> REJECT_REQUEST         malformed, stale, or impossible request

SNAPSHOT_REQUEST
  -> RECONCILE_RESULT       preserve a stable board and compare prior result

RECONCILE_RESULT
  -> GENERATE               history is updated; a new decision is needed
  -> REJECT_REQUEST         surface result contradicts the accepted move

GENERATE
  -> ANTICIPATE             one legal candidate is available
  -> ERROR                  no legal candidate / terminal position

ANTICIPATE
  -> EVALUATE               one candidate or one bounded continuation projected
  -> ANTICIPATE             more projection work remains

EVALUATE
  -> SELECT                 all candidates have a policy value

SELECT
  -> DECISION                a deterministic best move is retained

DECISION
  -> WAIT_RESULT            data words written first, READY written last

WAIT_RESULT
  -> WAIT_REQUEST           a new, valid sequence arrives
```

`ANTICIPATE` performs bounded projection: it applies a candidate move to the
immutable requested position, including captures, promotion, and jump
continuation.  A later version can project an opponent reply and a bounded
reply-to-reply line.  It must preserve the projected transition trace, not
only a numerical score: the trace explains why the policy selected a move and
allows the next observed board to be reconciled with it.

Each invocation advances at most one bounded unit of work: for example,
generate one piece's moves, apply one candidate, or score one projected line.
The mailbox remains `BUSY` until `DECISION`.  This is required because
the same Teensy must service cartridge reads within their timing window; a
search may never occupy a bus response path.

## Correlation with `ideal`, branches 01--04

The following is a design correlation, not a claim that the Checkers firmware
will copy the C# implementation.  The reference was read from the local
repository at `/home/cartheur/ame/aiventure/aiventure-github/cartheur/ideal/`.
Its branches are a useful conceptual progression.  The inspected branch tips
were `8fd3254` (01), `b9a6a25` (02), `96300ae` (03), and `d2f5248` (04).

| `ideal` branch | Evidence in its code | Checkers consequence |
| --- | --- | --- |
| `01-embodied-paradigm` | `Existence/Existence010.cs` retains `Experiences`, `Results`, and `Interactions`; `Step()` predicts before it obtains and records a result. | Retain the move/result interaction and compare it with the next board supplied by the surface.  The board request is not merely passive input; it is the result of the preceding enacted move. |
| `02-interactional-motivation` | `Existence/Existence020.cs` gives interactions a valence and changes behaviour from the enacted interaction. | Keep strategic preference distinct from legality and control.  Checkers policy values projected interactions; it does not replace validation or the FSA. |
| `03-constructivist-epistemology` | `Existence/Existence030.cs` learns a composite `preInteraction`/`postInteraction`, activates it from the last enacted interaction, and forms anticipations before selection. `Agent/Anticipation030.cs` ranks an anticipated interaction. | Preserve temporal context and generate projections from it.  At minimum, forced continuation and prior move must be remembered; later, the agent may retain recurring tactical sequences as explicit transition records. |
| `04-self-programming` | `Existence/Existence040.cs` creates composite interactions, associates abstract experiments with them, recursively `Enact`s their components, and aborts a sequence when a prerequisite enactment differs from what was intended. | Represent a multi-jump or bounded planned line as an intended sequence.  After every hop, reconcile the observed result; if it differs, stop that anticipation and re-enter validation rather than blindly finish the plan. |

Two details from Branch 04 are especially important.  First, selecting an
abstract/composite interaction is not the same thing as successfully enacting
it: the `Enact()` routine checks the pre-interaction before proceeding to the
post-interaction.  Second, Branch 04's selection uses anticipated structure
before recursive enactment.  That is the model for a Checkers agent that has
memory and anticipation rather than a sequence of disconnected if-statements.

The `ideal` project notes also state that there cannot be an anticipation
without a prerequisite interaction.  Applied here: do not invent a tactical
memory entry from a board diagram alone.  Record a transition only after a
move was proposed and the game surface returned an observed outcome.

## Implementation order

1. Keep the Stage 0 mailbox loopback in `NEXT-STEPS.md`; it validates
   transport, not intelligence.
2. Define `AgentMemory` and the FSA enum alongside the mailbox service.
   Implement `WAIT_REQUEST -> SNAPSHOT_REQUEST -> DECISION` first.
3. Add board validation and `RECONCILE_RESULT`, with serial traces that show
   request sequence, intended move, expected board hash, and observed board
   hash.
4. Add legal move generation, then one-step projection.  The initial policy
   may choose the lowest-numbered legal move, but only after it has passed
   through the projection state.
5. Add mandatory multi-jump as a composite intended interaction.  Only then
   add two-ply anticipation and more sophisticated policy terms.

Every fixture should specify the initial board, previous interaction when
relevant, candidate projections, selected move, expected resulting board, and
the trace of FSA phases.  This gives us a way to test memory, anticipation,
and strategy independently.
