#!/usr/bin/env bash
set -e
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$ROOT/build"
LOG="${TMPDIR:-/tmp}/shell-all.jsonl"
cd "$ROOT"
cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD" --config Release
"$BUILD/shell-emulator" --config "$ROOT/scripts/config.xml" --vfs cli-vfs \
    --log "$LOG" --script "$ROOT/scripts/startup_demo.txt" < /dev/null
cat "$LOG"
