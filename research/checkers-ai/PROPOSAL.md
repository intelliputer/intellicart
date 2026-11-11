# Checkers FSA proposal

## Goal

Build a deterministic Checkers opponent that runs on the Teensy, with a
purpose-built CP1610 game surface as its client.  It is a finite-state agent
over a bounded board representation: no LLM, neural network, network service,
or dependency on the original Mattel ROM.

The original Checkers disassembly remains a reference for a later integration
effort.  The first prototype proves the console-to-Teensy protocol using a
new, small test surface.

## Architecture

Intellicart already maps a configured RAM window to the Teensy's `RAM[]`
buffer.  CP1610 writes to that range reach the Teensy; reads return the
Teensy's current words.  This provides a mailbox without new hardware.

```text
CP1610 test surface                       Teensy FSA
-------------------                       ----------
draw board and accept player move  <-->   mapped-RAM mailbox
write board + command              ---->  validate and select move
read response + move               <----  response, reason, trace data
apply and render move
```

## Test-card strategy

Use one dedicated FAT/FAT32 SD card per experiment.  A card contains only the
test ROM, matching CFG, fixtures, expected results, and a result log.  This
keeps experimental mappings isolated from the normal game card.

```text
CHECKERS-FSA-T01/
├── CHECKERS_FSA_T01.bin
├── CHECKERS_FSA_T01.cfg
├── fixtures.txt
└── README.txt
```

The game-specific CFG reserves one RAM mailbox window.  `$D000` is the first
candidate, subject to Stage 0 validation of the exact `[memattr]` syntax and
confirmation that it does not overlap the test ROM mapping.

## Mailbox ABI v1

The CP1610 writes all request data first and commits by writing `command`.
The Teensy writes move data first and commits by writing `response`.  Both
sides use a sequence number to reject stale replies.

| Offset | Direction | Contents |
| --- | --- | --- |
| `+0` | CP1610 → Teensy | magic `$4348` (`CH`) |
| `+1` | CP1610 → Teensy | version and command |
| `+2` | CP1610 → Teensy | request sequence |
| `+3..+4` | CP1610 → Teensy | human men bitboard, low/high |
| `+5..+6` | CP1610 → Teensy | agent men bitboard, low/high |
| `+7..+8` | CP1610 → Teensy | kings bitboard, low/high |
| `+9` | CP1610 → Teensy | side to move / jump continuation |
| `+10` | Teensy → CP1610 | response status + echoed sequence |
| `+11` | Teensy → CP1610 | origin square (0–31) |
| `+12..+15` | Teensy → CP1610 | landing path, terminated by `$FFFF` |
| `+16` | Teensy → CP1610 | policy score / diagnostic reason |

## FSA

```text
IDLE → VALIDATE → GENERATE_MOVES → MUST_CAPTURE?
                                  ├─ yes → SCORE_CAPTURES
                                  └─ no  → SCORE_QUIET_MOVES
     → SELECT_TIE_BREAK → EMIT_MOVE → WAIT_FOR_NEXT_SEQUENCE
```

The initial fixed policy is:

1. Reject invalid encodings.
2. Generate every legal move.
3. Enforce compulsory captures.
4. Prefer the longest capture path.
5. Prefer promotion to king.
6. Avoid an immediately capturable landing piece where possible.
7. Prefer central squares, then the lowest square index as a stable tie break.

A fixed-depth alpha-beta search may be added later, but it is not needed to
validate the protocol or the FSA.

## Staged implementation

| Stage | Deliverable | Hardware acceptance |
| --- | --- | --- |
| 0 | Mailbox loopback ROM + CFG | CP1610 writes a sequence and receives it plus a known word. |
| 1 | Board encoder/display | Fixture board on TV matches mailbox bitboards. |
| 2 | Teensy legal-move generator | Fixture move sets match expected results. |
| 3 | Single-turn FSA | Each fixture receives its documented move. |
| 4 | Full turn exchange | Surface applies, renders, and acknowledges normal moves. |
| 5 | Multi-jumps and kings | Forced-capture and promotion fixtures pass on hardware. |
| 6 | Playable prototype | A complete human-versus-FSA game produces a trace. |

## Evidence to retain

For every card, retain the ROM/CFG revision, named fixtures, expected move,
Teensy serial trace, hardware result log, and recovery steps for returning to
the standard Intellicart card and firmware.

## Next action

Implement Stage 0 only: a mailbox-loopback ROM and its dedicated test SD
card.  It isolates mapper and synchronization risk before introducing game
rules, graphics, or agent policy.
