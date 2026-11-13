# Intellivision Programming I & II training set

This is a compact, source-grounded instruction-tuning set derived from the two
Elektronite IntyLab white papers already present in `programming/`:

- `Intellivision-Programming-I.pdf` (Paper #2, 9 pages)
- `Intellivision-Programming-II.pdf` (Paper #3, 13 pages)

`train.jsonl` uses the chat-completions JSONL convention: one object per line,
with a `messages` array.  The extra `metadata` object makes every example
traceable to a paper and PDF page.  Answers are concise paraphrases, with
assembly snippets included only where they clarify an operation.

The set contains 48 examples: CP1610 architecture, registers and addressing
modes from Paper I; then ROM setup, the EXEC, STIC display concepts, BACKTAB,
and the annotated Hello World program from Paper II.

## Use

Validate the file with:

```sh
jq -c . programming/intellivision-programming-training/train.jsonl >/dev/null
```

Keep `metadata.source_file` and `metadata.source_pdf_page` when converting to a
different training format.  They are useful for attribution, review, and
holding out source sections for evaluation.  Do not treat historical claims in
the papers as documentation for modern toolchains without separate validation.

## Coverage map

| Source | Pages | Topics |
| --- | --- | --- |
| Programming I | 3–6 | CP1610, registers, addressing modes, EXEC/JzIntv |
| Programming I | 7–9 | instruction families and timing notes |
| Programming II | 3–6 | comments, ROMW/ORG, ROMHDR, STIC setup |
| Programming II | 7–11 | BACKTAB, card values, `MVO@`, title and main routines |
| Programming II | 12–13 | complete exercise listing |

