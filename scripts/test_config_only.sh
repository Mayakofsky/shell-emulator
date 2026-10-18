#!/usr/bin/env bash
set -e
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$ROOT/build"
cd "$ROOT"
cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD" --config Release
bash "$ROOT/scripts/make_vfs.sh"
"$BUILD/shell-emulator" --config "$ROOT/scripts/config.xml" < /dev/null
cat "$ROOT/data/config-log.jsonl"
