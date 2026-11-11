# Portable Checkers FSA core

The Checkers agent should be one portable ISO C++11 library, not Arduino
logic with a desktop test program that merely resembles it.  The same source
must run in these three places:

```text
fixture runner ── direct calls ───────────┐
Mira ── CP1610 mailbox / peripheral ──────┼──> checkers_core (C++11)
Teensy ── mapped-RAM mailbox adapter ─────┘
```

The core owns Checkers rules, interaction memory, the FSA, anticipation, and
policy.  Each environment owns only transport and scheduling.

## Language and portability boundary

Target **ISO C++11**, which is supported by a normal Linux compiler, Mira's
current C++11 build, and the Teensy 4.1 Arduino toolchain.

The core must not include `Arduino.h`, Mira headers, SDL, serial I/O, file
I/O, clocks, threads, heap allocation, exceptions, RTTI, or `String`.  Use
fixed-capacity storage instead:

```text
std::array<Move, MAX_MOVES>
std::array<Projection, MAX_MOVES>
uint8_t count
enum class Phase : uint8_t
```

This makes memory use measurable on the Teensy and makes fixture execution
bit-for-bit repeatable on Linux.  It also avoids using C++17 facilities such
as `std::optional`; use an explicit `bool valid` beside a value instead.

Suggested source layout:

```text
research/checkers-ai/checkers-core/
├── include/checkers/
│   ├── board.hpp             # 32-square representation and validation
│   ├── move.hpp              # fixed-capacity move and jump path
│   ├── rules.hpp             # legal move generation and applyMove
│   ├── agent.hpp             # AgentMemory and one-step FSA API
│   ├── policy.hpp            # deterministic strategy interface
│   └── mailbox_abi.hpp       # word layout and encode/decode only
├── src/
│   ├── board.cpp
│   ├── rules.cpp
│   ├── agent.cpp
│   └── policy.cpp
├── tests/
│   └── fixture_runner.cpp    # native executable, no emulator dependency
└── CMakeLists.txt
```

The initial public API should be intentionally small:

```cpp
namespace checkers {

struct MailboxRequest { /* decoded words, sequence, board */ };
struct MailboxResponse { /* status, sequence, move, diagnostic */ };

class Agent {
public:
    void reset();
    void acceptRequest(const MailboxRequest& request);
    bool step();                 // performs one bounded FSA unit of work
    bool responseReady() const;
    MailboxResponse response() const;
};

} // namespace checkers
```

`step()` does no I/O.  The caller controls how often it is invoked.  A return
value of `true` means that one bounded unit was completed, not necessarily
that a move is ready.

## Adapters

### Native fixture adapter

The fixture runner decodes named positions, calls `acceptRequest()`, then
calls `step()` until `responseReady()`.  It verifies the legal move set,
chosen move, expected projected board, and phase trace.  This is the primary
place to test Checkers rules and policy; it is faster and more precise than a
screen-level emulator test.

### Teensy adapter

The Arduino/Teensy code remains responsible for the existing cartridge bus
timing.  At a safe point it:

1. snapshots the configured `RAM[]` mailbox after the CP1610 commits a
   command;
2. converts words with `mailbox_abi.hpp` and calls `Agent::acceptRequest()`;
3. calls `Agent::step()` once per safe service opportunity; and
4. copies the completed response words into `RAM[]`, writing `READY` last.

The core never accesses `RAM[]` directly.  This preserves the tested
Intellicart bus handler and lets the adapter enforce the BUSY/READY protocol.

### Mira adapter

Mira already loads `.bin` + `.cfg` images and has an Intellicart peripheral
with writable memory pages.  Therefore it can run the CP1610 test surface and
validate its drawing, controller handling, mailbox addresses, and CP1610-side
protocol.

However, Mira's ordinary Intellicart memory is a local cartridge image
(`icartrom_t.image`), not the physical Teensy's live `RAM[]` service.  A
normal Mira run will not cause the native agent to process a command or turn
BUSY into READY.  To test the complete protocol, add a small **host-only
Checkers peripheral** to Mira:

```text
CP1610 write to mailbox
        ↓
Mira CheckersMailboxPeripheral::write()
        ↓
checkers::Agent::acceptRequest()
        ↓
Mira scheduler / peripheral tick calls Agent::step()
        ↓
peripheral exposes response words on CP1610 reads
```

Mira's `mira/periph/periph.h` defines the read, write, and tick model, and
`mira/icart/icart.h` shows how its Intellicart registers itself on that bus.
The host peripheral should use that same model, be registered only for the
Checkers test configuration, and own a mailbox word array with the exact ABI
defined in `mailbox_abi.hpp`.

Do **not** modify Mira merely to test the rules: use the native fixture runner
first.  Add the peripheral only when the Stage 0 CP1610 surface exists and we
need an end-to-end emulator loop.  It should remain an optional Mira patch or
feature flag, never a requirement for building the normal emulator.

## Test ladder

```text
native C++ fixtures
  -> Mira: CP1610 surface + host Checkers mailbox peripheral
  -> real CP1610 + Teensy mapped-RAM mailbox
```

All three levels consume the same fixture data and mailbox ABI.  The latter
two test transport and presentation; neither substitutes for rule tests.

## Immediate implementation sequence

1. Create the portable core with board encoding, validation, and fixture
   runner only.
2. Add legal move generation and one-step deterministic selection; run native
   fixtures.
3. Define the word-exact mailbox ABI header shared by the core and Teensy
   adapter.
4. Assemble the Stage 0 CP1610 surface; run it in unmodified Mira to validate
   the screen and writable RAM map.
5. Add the optional Mira mailbox peripheral for end-to-end automated traces.
6. Add the Teensy adapter and validate the same traces on hardware.
