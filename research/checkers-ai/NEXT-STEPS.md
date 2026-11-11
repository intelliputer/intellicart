# Checkers AI: second-step implementation plan

> **Status — 11 November 2026:** The portable C++11 foundation now exists in
> [`research/checkers-fsa/checkers-core/`](../checkers-fsa/checkers-core/).
> It builds and passes native Stage-0 `PING` fixtures. Active implementation
> sequencing, acceptance criteria, and the next coding task now live in
> [`research/checkers-fsa/NEXT-STEPS.md`](../checkers-fsa/NEXT-STEPS.md).
> This document remains the original hardware-oriented plan and protocol
> rationale.

This is the plan after the proposal.  The objective is to validate the
mailbox transport before building a playable Checkers opponent.

## Constraint to design around

The Teensy is actively serving the Intellivision cartridge bus.  It must
produce mapped-RAM reads with tight timing, so it must not stop inside a bus
cycle to run a long move search.

The protocol therefore uses these response states:

```text
EMPTY → REQUESTED → BUSY → READY | ERROR
```

The CP1610 writes a complete request and then polls `response`.  The Teensy
returns `BUSY` until it has produced the response.  The first FSA is kept
small enough to advance incrementally between bus service events.  Any later
search must be incremental too; it may never delay a required bus response.

## Step 0 — capture the current baseline

**Portable-core status:** complete for the software baseline. The current core
defines the mailbox ABI, validates `PING`, preserves FSA sequencing, and has a
native fixture runner. The outstanding hardware-baseline items below remain
required before flashing experimental firmware.

1. Keep a copy of the committed Intellicart firmware HEX and the normal SD
   card available as recovery media.
2. Create a fresh, dedicated FAT/FAT32 test card labelled `CHECKERS-FSA-T01`.
3. Record the Teensy firmware commit, board revision, console, controller,
   and test-card contents in that card's `README.txt`.
4. Do not place the experiment on the everyday game card.

**Exit criterion:** the recovery firmware and normal card are immediately
available if the experimental mapper fails.

## Step 1 — validate a per-game RAM mapping

**Refined order:** first add the desktop mailbox harness and board-validation
fixtures in the portable core. Then assemble this CP1610 loopback surface and
run it in Mira before using a dedicated physical SD card.

1. Create a minimal CP1610 ROM that only renders `MAILBOX T01` and reads and
   writes a small mailbox window.
2. Create its same-named CFG with one ROM mapping and one `[memattr]` RAM
   range.  Start with a 32-word candidate window at `$D000` and adjust only
   after confirming the parser's accepted syntax and no address overlap.
3. On the TV, show a fixed word written to the mailbox and the value read
   back from it.
4. On the Teensy serial console, log the physical address and every write to
   the proposed RAM range.

**Pass condition:** a CP1610 write appears in the serial trace and the exact
same word is read back and displayed.  This proves the mapper and direction
of the existing `RAM[]` path without any agent logic.

## Step 2 — add a committed request/response handshake

**ABI refinement:** the portable core's `mailbox_abi.hpp` is now the single
source of truth. It packs protocol version/command in word `+1`, response
status/echoed sequence in word `+10`, and reserves words `+12..+23` for up to
12 landing squares (not `+12..+15`, which would be too short for all legal
multi-jump paths). Response data is written before the final `READY` commit.

Implement only the following fields at first:

| Word | Writer | Value |
| --- | --- | --- |
| `+0` | CP1610 | magic `$4348` |
| `+1` | CP1610 | monotonically increasing sequence |
| `+2` | CP1610 | command `PING` (written last) |
| `+10` | Teensy | `BUSY`, then `READY` with the sequence |
| `+11` | Teensy | known diagnostic word, for example `$A11E` |

Firmware work:

1. Define the mailbox base, word count, status constants, and a request
   structure in one clearly named section of the Teensy firmware.
2. Detect a write to `command`; validate magic and sequence before accepting
   a request.
3. Write `BUSY` immediately, then write data and finally `READY`.
4. Ignore malformed requests and set `ERROR`; never index outside `RAM[]`.
5. Add concise serial trace lines only behind a debug flag, so normal bus
   timing remains unaffected.

**Pass condition:** the CP1610 screen reports the matching sequence and
diagnostic word for at least 100 repeated requests, with no black screen,
bus lockup, or stale response.

## Step 3 — make servicing incremental

**Core status:** the portable agent already advances one FSA phase per
`step()` call. The future Teensy adapter must call that core operation only at
a safe point; it must never embed strategy or move generation in the bus read
path.

1. Add a small `serviceCheckersMailbox()` state machine.
2. Call it only at a safe point in the existing bus handler, never during the
   data-output window for a cartridge read.
3. Give it a bounded unit of work per invocation: validate one field, scan
   one piece, or score one candidate move.
4. Keep `BUSY` visible until all output words have been written.
5. Measure request-to-ready latency in Teensy serial output and retain it in
   the test-card log.

**Pass condition:** repeated polling remains stable while the service machine
advances and eventually returns `READY`.

## Step 4 — board transport diagnostic

1. Add the four bitboards and side-to-move word from `PROPOSAL.md`.
2. Create three named fixture positions: opening layout, one forced capture,
   and one king move.
3. Have the CP1610 surface render the submitted bitboards in a simple stable
   GROM/color-stack board view.
4. Have the Teensy validate piece overlap, range, and side-to-move fields.

**Pass condition:** the TV board, mailbox words, and serial trace agree for
all fixtures.

## Step 5 — first FSA move

**Refined order:** board validation and native legal-move fixtures precede
this step. The first move policy remains deliberately narrow and deterministic;
policy growth follows only after the rules fixtures pass.

Implement a deliberately narrow agent:

1. Generate mandatory single captures.
2. If no capture exists, generate ordinary forward moves for men.
3. Select the lowest-numbered legal move as the deterministic tie break.
4. Return origin and landing square only.

This small policy proves the end-to-end turn without prematurely adding
multi-jump, king, or evaluation complexity.

**Pass condition:** every Stage 4 fixture produces its documented move, and
the CP1610 surface applies and displays it.

## Step 6 — grow rules and strength separately

Add one rule group per test-card revision:

1. multi-jump continuation;
2. king movement and promotion;
3. capture-length preference;
4. safety and centre-square scoring; and
5. optional bounded alpha-beta search.

Each addition needs fixture tests, a serial trace, and a hardware pass before
the next group begins.

## Deliverables to create next

```text
research/checkers-ai/
├── PROPOSAL.md                 # architecture and protocol
├── NEXT-STEPS.md               # this staged plan
├── t01-mailbox/                # minimal CP1610 loopback ROM source
├── fixtures/                   # board positions and expected responses
└── traces/                     # retained serial/hardware observations
```

The immediate implementation target is `t01-mailbox`: it contains no
Checkers rules.  Its sole job is to prove the RAM mapping and `PING` handshake
on actual hardware.
