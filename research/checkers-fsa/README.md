# Checkers FSA implementation workspace

Begin implementation here.

```text
research/checkers-fsa/
└── checkers-core/       portable C++11 core and native fixtures
```

The separate [checkers-ai](../checkers-ai/) directory remains the research and
design record: FSA model, pseudocode, `ideal` correlation, and staged plan.
This directory is for code that will run unchanged in a native fixture runner,
a future Mira host peripheral, and a future Teensy adapter.

## First command

```sh
cmake -S research/checkers-fsa/checkers-core -B /tmp/checkers-core-build
cmake --build /tmp/checkers-core-build
ctest --test-dir /tmp/checkers-core-build --output-on-failure
```

The current vertical slice validates the versioned mailbox `PING` handshake.
The next implementation unit is board validation, followed by legal-move
fixtures; neither belongs in the emulator or Arduino adapter.
