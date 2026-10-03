#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

mkdir -p "$SOURCES"
cd "$SOURCES"

echo "========================================"
echo "Downloading Wayland $WAYLAND_VER"
echo "========================================"

wget -nc \
    "https://gitlab.freedesktop.org/wayland/wayland/-/releases/$WAYLAND_VER/downloads/wayland-${WAYLAND_VER}.tar.xz"

if [ ! -d "$SOURCES/wayland-$WAYLAND_VER" ]; then
    echo "Extracting Wayland..."
    tar -xf "wayland-${WAYLAND_VER}.tar.xz"
else
    echo "Wayland-$WAYLAND_VER already extracted."
fi

echo
echo "Wayland source:"
ls -ld "$SOURCES/wayland-$WAYLAND_VER"

echo
echo "Wayland download completed."
