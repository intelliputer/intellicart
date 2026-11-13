# CP-1600 Cross Assembler/Simulator training set

This source-grounded instruction-tuning set is derived from
`../CP-1600_Cross_Assembler_Simulator_Users_Manual_Nov74.pdf`, the Series 1600
Cross Assembler/Simulator Users Manual (66 PDF pages).

`train.jsonl` is chat-completions JSONL: one training example per line with
`system`, `user`, and `assistant` messages. Each record includes the source
PDF page and manual section in `metadata`. Answers are concise paraphrases,
not a transcription of the manual. They describe the historical S16XAL tool
suite; they are not instructions for a modern assembler unless independently
verified.

Coverage includes S16XAL source syntax and directives; S16LNK; S16SIM setup,
execution, tracing, and simulated I/O; S16XRF; S16BPT; and S16RTG.

Validate with:

```sh
jq -c . programming/cp1600-cross-assembler-training/train.jsonl >/dev/null
```

