#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

SRC_DIR="$SOURCES/systemd-$SYSTEMD_VER"
URL="https://github.com/systemd/systemd/archive/refs/tags/v$SYSTEMD_VER.tar.gz"

mkdir -p "$SOURCES"

if [ -d "$SRC_DIR/.git" ] || [ -f "$SRC_DIR/meson.build" ]; then
    echo "systemd $SYSTEMD_VER source already exists:"
    echo "  $SRC_DIR"
    exit 0
fi

ARCHIVE="$SOURCES/systemd-$SYSTEMD_VER.tar.gz"

echo "========================================"
echo "Downloading systemd $SYSTEMD_VER"
echo "========================================"
echo
echo "URL:"
echo "  $URL"
echo

rm -f "$ARCHIVE"
rm -rf "$SRC_DIR"

curl -L \
    "$URL" \
    -o "$ARCHIVE"

echo
echo "Extracting..."
echo

tar -xzf "$ARCHIVE" -C "$SOURCES"

echo
echo "========================================"
echo "systemd source ready"
echo "========================================"
echo
echo "Source directory:"
echo "  $SRC_DIR"
echo
echo "libudev source:"
echo "  $SRC_DIR/src/libudev"
echo