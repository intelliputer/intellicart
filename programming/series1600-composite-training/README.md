# Series 1600 composite training set

`train.jsonl` combines the refined source-grounded records from:

- `../intellivision-programming-training/train.jsonl` — 48 examples
- `../cp1600-cross-assembler-training/train.jsonl` — 66 examples

Each record has exactly one `system`, `user`, and `assistant` message. The
composite adds `metadata.dataset` and preserves `source_file`,
`source_pdf_page`, and (for the cross-assembler manual) `section`.

`evaluation.jsonl` is a deliberately disjoint 12-item smoke-test set. Do not
merge it into training data. It is intended to catch basic recall, comparison,
and applied-assembly failures after any model or prompt change.

Validate both files:

```sh
jq -c . programming/series1600-composite-training/train.jsonl >/dev/null
jq -c . programming/series1600-composite-training/evaluation.jsonl >/dev/null
```

