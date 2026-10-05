#!/usr/bin/env bash
set -euo pipefail

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
rom="$script_dir/../../rom/Checkers (1979) (Mattel).int"
output="$script_dir/checkers-words.lst"

{
  printf '; Checkers (1979) (Mattel) -- lossless big-endian CP1610 word listing\n'
  printf '; ROM base: $5000; one line per 16-bit word\n\n'
  od -An -v -tx2 -w2 "$rom" | awk '\''BEGIN { address = 0x5000 } { printf "$%04X: $%s\\n", address, $1; address++ }'\''
} > "$output"

printf 'Wrote %s\n' "$output"
