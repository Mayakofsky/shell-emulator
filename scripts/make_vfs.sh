#!/usr/bin/env bash
set -e

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
SAMPLES="$ROOT/data/vfs_samples"
OUTPUT="$ROOT/data/vfs"

mkdir -p "$OUTPUT"

pack() {
    local name="$1"
    local source="$SAMPLES/$name"
    local target="$OUTPUT/$name.zip"
    if [ ! -d "$source" ]; then
        echo "make_vfs: missing sample directory: $source" >&2
        return 1
    fi
    rm -f "$target"
    if command -v zip >/dev/null 2>&1; then
        ( cd "$source" && zip -qr "$target" . )
    elif command -v python3 >/dev/null 2>&1; then
        python3 - "$source" "$target" <<'PY'
import os, sys, zipfile
source, target = sys.argv[1], sys.argv[2]
with zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as archive:
    for root, _, files in os.walk(source):
        for name in files:
            full = os.path.join(root, name)
            rel = os.path.relpath(full, source)
            archive.write(full, rel.replace(os.sep, '/'))
PY
    else
        echo "make_vfs: neither 'zip' nor 'python3' is available" >&2
        return 1
    fi
    echo "make_vfs: created $target"
}

pack minimal
pack several
pack nested

rm -f "$OUTPUT/corrupt.zip"
printf 'this is not a zip archive\n' > "$OUTPUT/corrupt.zip"
echo "make_vfs: created $OUTPUT/corrupt.zip"
