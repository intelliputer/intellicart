# Portable Checkers core

This is the shared, freestanding C++11 foundation for the Checkers FSA.  It
builds as a Linux fixture executable today and is designed to be compiled into
the Teensy firmware later.  A Mira host peripheral will use the same library.

## Non-negotiable boundary

The core contains no Arduino, Teensy, Mira, SDL, file, serial, clock, heap,
exception, RTTI, or thread dependency.  It owns the mailbox protocol, FSA
state, and eventually all Checkers rules and policy.  Adapters own hardware
or emulator I/O and decide when to call `Agent::step()`.

## Build and test

Keep build products outside the source tree:

```sh
cmake -S research/checkers-fsa/checkers-core -B /tmp/checkers-core-build
cmake --build /tmp/checkers-core-build
ctest --test-dir /tmp/checkers-core-build --output-on-failure
```

## First vertical slice

The current implementation is deliberately Stage 0 only:

1. Decode a committed `PING` request from mailbox words.
2. Validate magic, version, command, and sequence.
3. Advance the FSA through bounded steps.
4. Emit a deterministic `READY` response with diagnostic word `$A11E`.

It does not yet generate Checkers moves.  A `BOARD` command is recognized by
the ABI but reports `UNIMPLEMENTED` until board validation and move generation
are introduced with fixtures.

## Rules module status

`include/checkers/rules.hpp` now implements the first rules-only increment:
uncrowned men make forward diagonal quiet moves or single forward captures;
any available capture suppresses every quiet move. Moves are returned in stable
origin/left/right order and `applyMenMove()` rejects anything outside that
generated set.

The module is intentionally incomplete in two visible ways: a position that
already requires jump continuation returns `MULTI_JUMP_NOT_IMPLEMENTED`, and
applying a move that reaches a promotion row returns
`PROMOTION_NOT_IMPLEMENTED`. Those outcomes prevent the prototype from
silently playing an illegal partial game while kings, promotion, and multi-hop
captures are implemented in their own tested increments.

`include/checkers/mailbox_abi.hpp` is the normative word-level contract.  The
CP1610 assembly, DesktopTeensy/Mira peripheral, and Teensy adapter must use
that contract rather than duplicate offsets or bit packing.
