# Checkers finite-state player proposal

## Objective

Build a deterministic Checkers opponent that runs on the Teensy and is driven
by a purpose-built Intellivision game surface.  It is a finite-state agent
with bounded board memory and fixed rules—not an LLM and not a cloud service.

The first goal is not to modify the original Mattel Checkers ROM.  It is to
prove a reliable console-to-Teensy game protocol using a small, independently
testable Checkers surface.  The existing Checkers disassembly remains useful
later as a reference and possible integration target.

## Why Intellicart is a good host

Intellicart already maps a configurable RAM region from the CP1610 address
space to the Teensy's `RAM[]` buffer.  The bus handler returns reads from that
buffer and stores CP1610 writes to it.  A game surface can therefore publish a
board position to the Teensy and retrieve an agent-selected move without any
new physical hardware.

```text
CP1610 Checkers surface                  Teensy 4.1
-----------------------                  ----------
render board and accept move  <------>   mapped RAM mailbox
write position + command    ---------->  validate / choose move
read response + move        <----------  response + trace fields
apply and animate move
```

## Test-card model

Use one dedicated FAT/FAT32 microSD card per experiment.  Each card contains
only the ROM, its matching `.cfg` mapping file, test fixtures, and expected
results for that experiment.  This prevents a new mapper or mailbox change
from affecting the normal game card.

Suggested first card:

```text
CHECKERS-FSA-T01/
├── CHECKERS_FSA_T01.bin       # CP1610 diagnostic/game surface
├── CHECKERS_FSA_T01.cfg       # ROM and mailbox-RAM mapping
├── fixtures.txt               # named board positions and expected moves
└── README.txt                 # test sequence and observed result log
```

The game-specific CFG must reserve a mailbox RAM window and must be validated
with a loopback test before any Checkers logic is introduced.  `$D000` is a
reasonable candidate range, but the final address and exact `[memattr]`
syntax are Phase 0 deliverables; they must not overlap a ROM mapping.

## Mailbox protocol, version 1

The mailbox is a small, fixed, 16-bit-word ABI.  A sequence number prevents
the surface from accepting a stale response.

| Offset | Direction | Meaning |
| --- | --- | --- |
| `+0` | CP1610 → Teensy | magic (`$4348`, “CH”) |
| `+1` | CP1610 → Teensy | protocol version and command |
| `+2` | CP1610 → Teensy | request sequence number |
| `+3..+4` | CP1610 → Teensy | human men bitboard, low/high 16 bits |
| `+5..+6` | CP1610 → Teensy | agent men bitboard, low/high 16 bits |
| `+7..+8` | CP1610 → Teensy | kings bitboard, low/high 16 bits |
| `+9` | CP1610 → Teensy | side to move and forced-jump continuation |
| `+10` | Teensy → CP1610 | response status and echoed sequence |
| `+11` | Teensy → CP1610 | move origin square (0–31) |
| `+12..+15` | Teensy → CP1610 | landing-square path; `$FFFF` terminates it |
| `+16` | Teensy → CP1610 | diagnostic reason / policy score |

The CP1610 writes all request words first and writes the command word last.
The Teensy writes move data first and response status last.  This produces a
simple commit protocol without requiring interrupts or races.

## Agent definition

The player is a deterministic finite-state controller over a bounded board.
It may keep the board in the mailbox format and needs no learning model.

```text
IDLE → VALIDATE → GENERATE_MOVES → MUST_CAPTURE?
                                  ├─ yes → SCORE_CAPTURES
                                  └─ no  → SCORE_QUIET_MOVES
     → SELECT_TIE_BREAK → EMIT_MOVE → WAIT_FOR_NEXT_SEQUENCE
```

Initial policy, in priority order:

1. Reject invalid board encodings and return a diagnostic status.
2. Generate every legal move for the requested side.
3. Enforce compulsory captures.
4. Prefer the longest legal capture path.
5. Prefer promotion to king.
6. Prefer a move that does not allow immediate capture of its landing piece.
7. Prefer central squares, then a fixed lowest-square tie break.

This is sufficient for a credible first opponent.  A fixed-depth alpha-beta
search can be added later, but it remains optional and deterministic; the
protocol and legal-move generator must not depend on it.

## Implementation stages

| Stage | Deliverable | Hardware acceptance criterion |
| --- | --- | --- |
| 0 | Mailbox config and loopback ROM | CP1610 writes a sequence; Teensy returns the same sequence and a known word. |
| 1 | Board encoder and screen diagnostic | A fixture board displayed on TV matches the mailbox bitboards. |
| 2 | Teensy legal-move generator | Fixture suite produces exact legal move sets and rejects invalid positions. |
| 3 | Single-move FSA | Each fixture returns its documented deterministic move. |
| 4 | Full turn exchange | Surface applies, renders, and acknowledges a normal move. |
| 5 | Captures, multi-jumps, and kings | Forced-jump and promotion fixtures pass on hardware. |
| 6 | Playable prototype | Human versus FSA completes a full game with an exported move trace. |

## Evidence and regression artifacts

Every test card should preserve:

- ROM and matching CFG revision;
- fixture name, board encoding, side to move, and expected response;
- Teensy serial trace of command, sequence, legal-move count, and selected
  move;
- a pass/fail hardware log; and
- a recovery instruction pointing to the normal Intellicart SD card and
  known-good firmware.

## Non-goals for the first prototype

- Patching or redistributing the original Checkers ROM.
- Neural networks, LLM inference, or network connectivity.
- A strong tournament engine before the board protocol is reliable.
- Custom GRAM graphics; the first diagnostic surface should use the stable
  color-stack/GROM drawing path.

## Recommended next action

Implement Stage 0 as a minimal CP1610 mailbox-loopback ROM and one dedicated
test SD card.  It isolates the mapper/protocol risk before Checkers rules,
graphics, or FSA policy are introduced.
