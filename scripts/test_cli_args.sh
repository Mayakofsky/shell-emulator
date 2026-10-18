#!/usr/bin/env bash
set -e
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$ROOT/build"
LOG="${TMPDIR:-/tmp}/shell-cli-args.jsonl"
cd "$ROOT"
cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD" --config Release
"$BUILD/shell-emulator" --vfs "$ROOT/data/vfs" --log "$LOG" \
    --script "$ROOT/scripts/startup_demo.txt" --config "$ROOT/scripts/config.xml" < /dev/null
cat "$LOG"
