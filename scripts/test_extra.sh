#!/usr/bin/env bash
set -e
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$ROOT/build"
VFS_OUT="$ROOT/data/vfs"

bash "$ROOT/scripts/make_vfs.sh"
if [ ! -x "$BUILD/shell-emulator" ]; then
    echo "Build the project first: cmake --build $BUILD" >&2
    exit 1
fi
"$BUILD/shell-emulator" --vfs "$VFS_OUT/several.zip" \
    --script "$ROOT/scripts/startup_extra.txt" < /dev/null
