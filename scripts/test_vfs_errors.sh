#!/usr/bin/env bash
set -e
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$ROOT/build"
VFS_OUT="$ROOT/data/vfs"
LOG="${TMPDIR:-/tmp}/shell-vfs-errors.jsonl"

bash "$ROOT/scripts/make_vfs.sh"
if [ ! -x "$BUILD/shell-emulator" ]; then
    echo "Build the project first: cmake --build $BUILD" >&2
    exit 1
fi

echo "---- nonexistent archive ----"
set +e
"$BUILD/shell-emulator" --vfs "$VFS_OUT/does-not-exist.zip" \
    --script "$ROOT/scripts/startup_vfs.txt" < /dev/null
echo "exit code: $?"
echo "---- corrupt archive ----"
"$BUILD/shell-emulator" --vfs "$VFS_OUT/corrupt.zip" \
    --script "$ROOT/scripts/startup_vfs.txt" < /dev/null
echo "exit code: $?"
set -e
echo "---- runtime vfs errors with logging ----"
"$BUILD/shell-emulator" --vfs "$VFS_OUT/several.zip" --log "$LOG" \
    --script "$ROOT/scripts/startup_vfs_errors.txt" < /dev/null
cat "$LOG"
