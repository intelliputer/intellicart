# Series 1600 training-data fitness report

## Verdict

The composite set is fit for source-grounded prototyping, prompt regression,
and retrieval-style QA. It is not by itself fit for production fine-tuning of a
general programming assistant. Its historical domain is narrow, the corpus is
small, and its examples are mostly single-turn fact recall.

## Inventory and checks

| Asset | Records | Purpose |
| --- | ---: | --- |
| `intellivision-programming-training/train.jsonl` | 48 | CP1610 and introductory Intellivision programming |
| `cp1600-cross-assembler-training/train.jsonl` | 66 | S16XAL, S16LNK, S16SIM, and companion tools |
| `series1600-composite-training/train.jsonl` | 114 | Unified train corpus with dataset provenance |
| `series1600-composite-training/evaluation.jsonl` | 12 | Disjoint smoke-test set |

All training records have the expected three chat roles and parse as JSONL.
All source records now use the consistent `source_pdf_page` metadata field.
The CP-1600 page index was rechecked against the PDF; this corrects the
previous section/page drift (for example, `TLIM` is PDF page 27, not page 30).

## Strengths

- Every training response is concise, directly responsive, and grounded in a
  local source PDF.
- There are no exact duplicate prompts in either source set.
- The two sources complement each other: the Intellivision papers provide
  CP1610 and platform context, while the manual provides assembler and
  simulator workflow.
- The evaluation set tests application and comparison as well as recall, and
  is kept out of the combined training file.
- Per-record provenance supports manual spot checks and future correction.

## Remaining fitness limits

- **Scale:** 114 training examples is a seed corpus, not a sufficient
  fine-tuning corpus. Repetition and overfitting are likely.
- **Style diversity:** all records follow a nearly identical single-turn QA
  pattern and mostly share an instructional system message.
- **Task diversity:** there are few complete assembly-writing, diagnosis,
  repair, or explanation-of-output tasks. No example asks the model to decline
  unsupported claims or identify uncertainty in OCR-damaged text.
- **Source diversity:** the corpus is based on three introductory/historical
  documents. It should not be treated as comprehensive hardware or toolchain
  documentation.
- **Evaluation depth:** the 12-item set is a smoke test, not a statistically
  meaningful benchmark. It has no rubric for partial correctness, citation
  accuracy, or assembly compilation/execution.

## Fitness by use

| Use | Fitness | Rationale |
| --- | --- | --- |
| Retrieval-augmented QA prototype | Good | Provenance and focused answers are useful. |
| Prompt/regression smoke testing | Good | The held-out cases cover core concepts. |
| Narrow adapter/LoRA experiment | Conditional | Appropriate only with conservative training and manual review. |
| Production fine-tuning | Not ready | Corpus size, diversity, and evaluation are insufficient. |
| Authoritative hardware reference | Not suitable alone | Historical and OCR-derived sources require corroboration. |

## Recommended next gate

Before fine-tuning, expand to at least several hundred independently written,
source-cited examples, with roughly balanced categories: code completion,
code review, debugging, command use, and conceptual explanation. Add a
separate evaluation set with executable or emulator-checked assembly cases,
negative/uncertainty cases, and a review rubric that scores factual accuracy,
provenance, and safe handling of historical-tool ambiguity.

