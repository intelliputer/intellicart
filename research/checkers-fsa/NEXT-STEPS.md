# Checkers FSA next steps

The portable foundation is committed. This roadmap advances one independently
verifiable layer at a time; no Checkers rules are added to the Teensy or Mira
until they pass in the native fixture runner.

## Current baseline

- `checkers-core` builds as ISO C++11 on Linux.
- The versioned 32-word mailbox ABI is defined in one header.
- The FSA has a bounded Stage-0 `PING` request/response path.
- A desktop `MailboxHarness` drives the same word-level request/response path
  that the future Mira and Teensy adapters will use.
- Fixtures prove valid PING, bad magic/version/command, response ordering,
  acknowledgement, stale-sequence rejection, sequence wrap, and an explicit
  `UNIMPLEMENTED` response for `BOARD`.

## 1. Complete the protocol harness — complete

**Implement:** a desktop `MailboxHarness` around `checkers::Agent`. It owns a
32-word mailbox, writes a request in CP1610 order, invokes one `Agent::step()`
at a time, and asserts that response data appears before `READY`.

**Add fixtures:** bad version, bad command, sequence wrap boundary, response
acknowledgement, and an unimplemented `BOARD` request.

**Pass:** achieved. Every malformed request terminates as `ERROR`; no new
request is accepted before the prior response is acknowledged; `READY` never
appears before move and diagnostic words are complete.

## 2. Add a validated board model — complete

**Implement:** `Board::isValid()` and square/row helpers for the 32 playable
squares. Validate non-overlapping men, kings only on occupied squares, a legal
side-to-move, and a valid forced-jump continuation square.

**Add fixtures:** opening position, a king position, each invalid overlap, an
orphan king bit, and each invalid continuation encoding.

**Pass:** achieved for the current no-move boundary. The portable core now
uses the independently validated 32-square mapping, and a valid `BOARD`
request reaches its deliberate `UNIMPLEMENTED` boundary. Invalid board
encodings return `INVALID_BOARD` before the request is accepted as agent state.

## 3. Build the rules engine before policy

**Implement:** fixed-capacity generation for men: forward quiet moves, single
captures, compulsory-capture filtering, and `applyMove()`. Do not add search
or scores yet.

**Add fixtures:** opening legal moves, blocked man, edge squares, one forced
capture, two alternative captures, and no legal move.

**Pass:** generated moves exactly equal the fixture move sets. In particular,
when any capture exists, no quiet move survives.

## 4. First playable deterministic FSA

**Implement:** connect `GENERATE -> PROJECT -> EVALUATE -> SELECT` for one
ply. A projection is an immutable copy of the requested board after a legal
move. The initial policy is only: capture is compulsory, then choose the
lowest origin/landing sequence.

**Add fixtures:** one response per Stage-3 position, expected projected board,
and phase trace.

**Pass:** each valid `BOARD` request produces one reproducible `READY` move
and the expected board hash. This is the first end-to-end computer player.

## 5. Finish Checkers rules, one isolated group at a time

1. complete multi-jump paths;
2. promotion and kings;
3. forced continuation after a jump; and
4. draw/repetition bookkeeping.

Every group adds only its own fixtures and must leave all previous fixtures
unchanged. The twelve-word landing-path reservation in the ABI already avoids
a later protocol redesign for multi-jumps.

## 6. Grow anticipation and policy separately

**Anticipation:** add bounded opponent replies to projections, retaining the
transition trace.

**Policy:** add capture-length, promotion, safety, centre, and material terms
one at a time. Each policy change needs fixture positions where the new term
is the sole reason the selected move changes.

**Pass:** every selected move includes an inspectable projected line and
score breakdown. No policy term may alter move legality.

## 7. Connect the desktop environments

1. Implement `DesktopTeensyMailbox` using the portable core and run the same
   fixtures through the word-level ABI.
2. Assemble a minimal CP1610 `PING` surface using SDK1600.
3. Run that ROM in stock Mira to validate display, controls, and writable RAM
   mapping.
4. Add an optional Mira `periph`-bus mailbox peripheral that calls the same
   `Agent::step()` during its tick. Use it for CP1610-to-agent integration
   traces.

**Pass:** the CP1610 surface shows matching sequence and `$A11E` in Mira for
100 consecutive PINGs, then returns documented moves for fixture boards.

## 8. Connect hardware last

**Implement:** a thin Teensy adapter in Intellicart firmware. It snapshots
`RAM[]`, calls one core step only at a safe service point, then writes result
words before `READY`.

**Pass:** the dedicated test SD card repeats the Mira fixture trace on real
hardware without a black screen, missed bus response, or stale reply.

## Next coding task

Start with Step 3: generate legal moves for uncrowned men only. Begin with
forward quiet moves and then single captures; fixtures must prove that the
existence of any capture removes every quiet move. Do not add policy, kings,
multi-jumps, or the Teensy/Mira adapters in this increment.
