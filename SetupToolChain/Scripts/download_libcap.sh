#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

SRC_DIR="$SOURCES/libcap-$LIBCAP_VER"
URL="https://www.kernel.org/pub/linux/libs/security/linux-privs/libcap2/libcap-$LIBCAP_VER.tar.xz"
ARCHIVE="$SOURCES/libcap-$LIBCAP_VER.tar.xz"

mkdir -p "$SOURCES"

if [ -f "$SRC_DIR/Makefile" ]; then
    echo "libcap $LIBCAP_VER source already exists:"
    echo "  $SRC_DIR"
    exit 0
fi

echo "========================================"
echo "Downloading libcap $LIBCAP_VER"
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
echo "libcap source ready"
echo "========================================"
echo
echo "Source directory:"
echo "  $SRC_DIR"
echo
echo "Version:"
echo "  $LIBCAP_VER"
echo