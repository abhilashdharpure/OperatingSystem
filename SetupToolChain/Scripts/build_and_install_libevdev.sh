#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

SRC_DIR="$SOURCES/libevdev-$LIBEVDEV_VER"
BUILD_DIR="$TOP/build-libevdev"

export PKG_CONFIG_SYSROOT_DIR="$SYSROOT"
export PKG_CONFIG_LIBDIR="$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/share/pkgconfig:$SYSROOT/usr/lib64/pkgconfig"
unset PKG_CONFIG_PATH

if [ ! -d "$SRC_DIR" ]; then
    echo "ERROR: libevdev source directory not found:"
    echo "  $SRC_DIR"
    echo
    echo "Run:"
    echo "  $SCRIPT_DIR/download_libevdev.sh"
    exit 1
fi

rm -rf "$BUILD_DIR"

echo "========================================"
echo "Building libevdev $LIBEVDEV_VER"
echo "========================================"
echo
echo "Source:"
echo "  $SRC_DIR"
echo
echo "Build:"
echo "  $BUILD_DIR"
echo
echo "Sysroot:"
echo "  $SYSROOT"
echo

meson setup \
    "$BUILD_DIR" \
    "$SRC_DIR" \
    --cross-file "$MESON_CROSS_FILE" \
    --prefix=/usr \
    --buildtype=release \
    -Dtests=disabled \
    -Ddocumentation=disabled \
    -Dtools=enabled

echo
echo "========================================"
echo "Building"
echo "========================================"
echo

ninja -C "$BUILD_DIR"

echo
echo "========================================"
echo "Installing into sysroot"
echo "========================================"
echo

DESTDIR="$SYSROOT" ninja -C "$BUILD_DIR" install

echo
echo "========================================"
echo "libevdev installation complete"
echo "========================================"
echo

echo "Installed files:"
echo

find "$SYSROOT/usr" \
    \( -name 'libevdev*' -o -name 'libevdev.pc' \) \
    -print | sort