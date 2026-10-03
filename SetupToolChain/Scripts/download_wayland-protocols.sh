#!/bin/bash
set -e

source ./Scripts/env.sh

SRC_DIR="$SOURCES/wayland-protocols-$WAYLAND_PROTOCOLS_VER"
REPO="https://gitlab.freedesktop.org/wayland/wayland-protocols.git"

echo "========================================"
echo "Download Wayland Protocols"
echo "========================================"
echo "Version : $WAYLAND_PROTOCOLS_VER"
echo "Source  : $SRC_DIR"
echo

mkdir -p "$SOURCES"

if [ -d "$SRC_DIR" ]; then
    echo "Source directory already exists:"
    echo "  $SRC_DIR"
    echo
    echo "Skipping download."
    exit 0
fi

echo "Cloning wayland-protocols $WAYLAND_PROTOCOLS_VER..."

git clone \
    --branch "$WAYLAND_PROTOCOLS_VER" \
    --depth 1 \
    "$REPO" \
    "$SRC_DIR"

echo
echo "Wayland protocols source downloaded successfully:"
echo "  $SRC_DIR"
echo

cd "$SRC_DIR"

echo "Git commit:"
git rev-parse HEAD

echo
echo "Done."
