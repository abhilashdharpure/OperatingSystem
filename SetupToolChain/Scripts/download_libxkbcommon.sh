#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

mkdir -p "$SOURCES"
cd "$SOURCES"

echo "========================================"
echo "Downloading libxkbcommon $LIBXKBCOMMON_VER"
echo "========================================"

ARCHIVE="libxkbcommon-xkbcommon-${LIBXKBCOMMON_VER}.tar.gz"
SRC_DIR="$SOURCES/libxkbcommon-xkbcommon-${LIBXKBCOMMON_VER}"

wget -nc \
    "https://github.com/xkbcommon/libxkbcommon/archive/refs/tags/xkbcommon-${LIBXKBCOMMON_VER}.tar.gz" \
    -O "$ARCHIVE"

if [ ! -d "$SRC_DIR" ]; then
    echo "Extracting libxkbcommon..."
    tar -xf "$ARCHIVE"
else
    echo "libxkbcommon-$LIBXKBCOMMON_VER already extracted."
fi

echo
echo "libxkbcommon source:"
ls -ld "$SRC_DIR"

echo
echo "libxkbcommon download completed."