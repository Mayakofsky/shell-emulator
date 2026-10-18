#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" --config Release

if [[ "${1:-}" == "test" ]]; then
    ctest --test-dir "$BUILD_DIR" --output-on-failure
    exit $?
fi

"$BUILD_DIR/shell-emulator"
exit $?
