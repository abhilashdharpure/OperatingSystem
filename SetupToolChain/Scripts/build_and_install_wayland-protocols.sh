#!/bin/bash
set -e

source ./Scripts/env.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

SRC_DIR="$SOURCES/wayland-protocols-$WAYLAND_PROTOCOLS_VER"
BUILD_DIR="$TOP/wayland-protocols-$WAYLAND_PROTOCOLS_VER"

echo "========================================"
echo "Build Wayland Protocols"
echo "========================================"
echo "Version : $WAYLAND_PROTOCOLS_VER"
echo "Target  : $TARGET"
echo "Source  : $SRC_DIR"
echo "Build   : $BUILD_DIR"
echo "Sysroot : $SYSROOT"
echo

# ------------------------------------------------------------
# Check source
# ------------------------------------------------------------

if [ ! -d "$SRC_DIR" ]; then
    echo "ERROR: Wayland protocols source not found:"
    echo "  $SRC_DIR"
    echo
    echo "Run:"
    echo "  ./Scripts/download_wayland-protocols.sh"
    exit 1
fi

# ------------------------------------------------------------
# Target pkg-config environment
# ------------------------------------------------------------

export PKG_CONFIG_SYSROOT_DIR="$SYSROOT"
export PKG_CONFIG_LIBDIR="$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/share/pkgconfig:$SYSROOT/usr/lib64/pkgconfig"
unset PKG_CONFIG_PATH

# ------------------------------------------------------------
# Clean previous build
# ------------------------------------------------------------

rm -rf "$BUILD_DIR"

mkdir -p "$TOP"

# ------------------------------------------------------------
# Meson configure
# ------------------------------------------------------------

echo "Configuring wayland-protocols..."

meson setup \
    "$BUILD_DIR" \
    "$SRC_DIR" \
    --cross-file "$MESON_CROSS_FILE" \
    --native-file "$SCRIPT_DIR/meson/native-wayland.ini" \
    --prefix=/usr \
    --buildtype=release \
    -Dtests=false

# ------------------------------------------------------------
# Build
# ------------------------------------------------------------

echo
echo "Building wayland-protocols..."

ninja -C "$BUILD_DIR"

# ------------------------------------------------------------
# Install
# ------------------------------------------------------------

echo
echo "Installing wayland-protocols..."

DESTDIR="$SYSROOT" ninja -C "$BUILD_DIR" install

echo
echo "========================================"
echo "Wayland Protocols installation complete"
echo "========================================"
echo

echo "Installed protocol files:"
find "$SYSROOT/usr/share/wayland-protocols" \
    -type f \
    | sort

echo
echo "Done."