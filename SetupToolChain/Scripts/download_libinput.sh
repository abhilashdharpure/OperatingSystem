#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

REPO="https://gitlab.freedesktop.org/libinput/libinput.git"
SRC_DIR="$SOURCES/libinput-$LIBINPUT_VER"

mkdir -p "$SOURCES"

if [ -d "$SRC_DIR/.git" ]; then
    echo "libinput $LIBINPUT_VER already exists:"
    echo "  $SRC_DIR"
    exit 0
fi

rm -rf "$SRC_DIR"

echo "Cloning libinput $LIBINPUT_VER..."

git clone \
    --branch "$LIBINPUT_VER" \
    --depth 1 \
    "$REPO" \
    "$SRC_DIR"

cd "$SRC_DIR"

echo
echo "libinput source:"
git describe --tags --always
echo
echo "Source directory:"
echo "  $SRC_DIR"