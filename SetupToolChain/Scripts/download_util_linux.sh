#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

SRC_DIR="$SOURCES/util-linux-$UTIL_LINUX_VER"
URL="https://www.kernel.org/pub/linux/utils/util-linux/v2.40/util-linux-$UTIL_LINUX_VER.tar.xz"
ARCHIVE="$SOURCES/util-linux-$UTIL_LINUX_VER.tar.xz"

mkdir -p "$SOURCES"

if [ -f "$SRC_DIR/configure" ]; then
    echo "util-linux $UTIL_LINUX_VER source already exists:"
    echo "  $SRC_DIR"
    exit 0
fi

echo "========================================"
echo "Downloading util-linux $UTIL_LINUX_VER"
echo "========================================"
echo
echo "URL:"
echo "  $URL"
echo
echo "Source:"
echo "  $SRC_DIR"
echo

rm -f "$ARCHIVE"
rm -rf "$SRC_DIR"

curl -L "$URL" -o "$ARCHIVE"

echo
echo "========================================"
echo "Extracting"
echo "========================================"
echo

tar -xJf "$ARCHIVE" -C "$SOURCES"

echo
echo "========================================"
echo "util-linux source ready"
echo "========================================"
echo
echo "Source directory:"
echo "  $SRC_DIR"
echo
echo "Version:"
echo "  $UTIL_LINUX_VER"
echo