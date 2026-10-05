#!/usr/bin/env bash
set -euo pipefail

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_dir=$(CDPATH= cd -- "$script_dir/../.." && pwd)
rom="$repo_dir/rom/Checkers (1979) (Mattel).int"
sdk_source="$repo_dir/../sdk1600/src/dasm/dasm1600.c"
native_dasm=$(mktemp)

trap 'rm -f "$native_dasm"' EXIT

if [[ ! -f "$sdk_source" ]]; then
  printf 'SDK1600 source not found: %s\n' "$sdk_source" >&2
  exit 1
fi

gcc -O2 -o "$native_dasm" "$sdk_source"
"$native_dasm" "$rom" "$script_dir/checkers.sym" > "$script_dir/checkers.asm"
printf 'Wrote %s\n' "$script_dir/checkers.asm"
