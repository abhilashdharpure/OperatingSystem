#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

REPO="https://gitlab.freedesktop.org/libevdev/libevdev.git"
SRC_DIR="$SOURCES/libevdev-$LIBEVDEV_VER"
TAG="libevdev-$LIBEVDEV_VER"

mkdir -p "$SOURCES"

if [ -d "$SRC_DIR/.git" ]; then
    echo "libevdev $LIBEVDEV_VER already exists:"
    echo "  $SRC_DIR"
    exit 0
fi

rm -rf "$SRC_DIR"

echo "Cloning libevdev $LIBEVDEV_VER..."

git clone \
    --branch "$TAG" \
    --depth 1 \
    "$REPO" \
    "$SRC_DIR"

cd "$SRC_DIR"

echo
echo "libevdev source:"
git describe --tags --exact-match HEAD

echo
echo "Source directory:"
echo "  $SRC_DIR"