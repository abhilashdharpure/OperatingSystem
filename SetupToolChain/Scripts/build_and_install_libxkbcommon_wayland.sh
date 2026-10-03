#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"


# ------------------------------------------------------------
# Target pkg-config
# ------------------------------------------------------------

export PKG_CONFIG_SYSROOT_DIR="$SYSROOT"
export PKG_CONFIG_LIBDIR="$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/share/pkgconfig:$SYSROOT/usr/lib64/pkgconfig"
unset PKG_CONFIG_PATH


# ------------------------------------------------------------
# Paths
# ------------------------------------------------------------

LIBXKBCOMMON_SRC="$SOURCES/libxkbcommon-xkbcommon-$LIBXKBCOMMON_VER"
BUILD_DIR="$TOP/build-libxkbcommon-wayland"


echo "========================================"
echo "Building libxkbcommon $LIBXKBCOMMON_VER"
echo "with Wayland support"
echo "========================================"
echo "Target  : $TARGET"
echo "Sysroot : $SYSROOT"
echo "Prefix  : $PREFIX"
echo "Source  : $LIBXKBCOMMON_SRC"
echo "Build   : $BUILD_DIR"
echo


# ------------------------------------------------------------
# Check source
# ------------------------------------------------------------

if [ ! -d "$LIBXKBCOMMON_SRC" ]; then
    echo "ERROR: libxkbcommon source not found:"
    echo "$LIBXKBCOMMON_SRC"
    echo
    echo "Run:"
    echo "  $SCRIPT_DIR/download_libxkbcommon.sh"
    exit 1
fi


# ------------------------------------------------------------
# Check compiler
# ------------------------------------------------------------

if [ ! -x "$PREFIX/bin/$TARGET-gcc" ]; then
    echo "ERROR: Cross compiler not found:"
    echo "$PREFIX/bin/$TARGET-gcc"
    exit 1
fi


# ------------------------------------------------------------
# Check Meson
# ------------------------------------------------------------

if ! command -v meson >/dev/null 2>&1; then
    echo "ERROR: Meson is not installed."
    exit 1
fi


# ------------------------------------------------------------
# Check Meson cross file
# ------------------------------------------------------------

if [ ! -f "$MESON_CROSS_FILE" ]; then
    echo "ERROR: Meson cross file not found:"
    echo "$MESON_CROSS_FILE"
    exit 1
fi


# ------------------------------------------------------------
# Check Wayland dependencies
# ------------------------------------------------------------

echo "========================================"
echo "Checking Wayland dependencies"
echo "========================================"

echo
echo "Wayland client:"
pkg-config --modversion wayland-client

echo
echo "Wayland server:"
pkg-config --modversion wayland-server

echo
echo "Wayland protocols:"
pkg-config --modversion wayland-protocols


# ------------------------------------------------------------
# Clean Wayland-enabled build
# ------------------------------------------------------------

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"


# ------------------------------------------------------------
# Meson configuration
# ------------------------------------------------------------

echo
echo "========================================"
echo "Configuring libxkbcommon"
echo "Wayland support: ENABLED"
echo "========================================"

meson setup "$BUILD_DIR" \
    "$LIBXKBCOMMON_SRC" \
    --cross-file "$MESON_CROSS_FILE" \
    --native-file "$SCRIPT_DIR/meson/native-wayland.ini" \
    --prefix=/usr \
    --buildtype=release \
    -Ddefault_library=both \
    -Denable-x11=false \
    -Denable-docs=false \
    -Denable-wayland=true


# ------------------------------------------------------------
# Build
# ------------------------------------------------------------

echo
echo "========================================"
echo "Building libxkbcommon"
echo "========================================"

ninja -C "$BUILD_DIR"


# ------------------------------------------------------------
# Install
# ------------------------------------------------------------

echo
echo "========================================"
echo "Installing libxkbcommon"
echo "========================================"

DESTDIR="$SYSROOT" ninja -C "$BUILD_DIR" install


# ------------------------------------------------------------
# Verify installed files
# ------------------------------------------------------------

echo
echo "========================================"
echo "Verifying libxkbcommon"
echo "========================================"

if [ ! -f "$SYSROOT/usr/lib/libxkbcommon.a" ]; then
    echo "ERROR: libxkbcommon.a was not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/libxkbcommon.so" ]; then
    echo "ERROR: libxkbcommon.so was not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/libxkbregistry.a" ]; then
    echo "ERROR: libxkbregistry.a was not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/libxkbregistry.so" ]; then
    echo "ERROR: libxkbregistry.so was not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/pkgconfig/xkbcommon.pc" ]; then
    echo "ERROR: xkbcommon.pc was not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/include/xkbcommon/xkbcommon.h" ]; then
    echo "ERROR: xkbcommon headers were not installed."
    exit 1
fi


# ------------------------------------------------------------
# Verify target pkg-config
# ------------------------------------------------------------

echo
echo "========================================"
echo "Checking target pkg-config"
echo "========================================"

echo
echo "xkbcommon version:"
pkg-config --modversion xkbcommon

echo
echo "xkbcommon CFLAGS:"
pkg-config --cflags xkbcommon

echo
echo "xkbcommon LIBS:"
pkg-config --libs xkbcommon


# ------------------------------------------------------------
# Verify Wayland integration
# ------------------------------------------------------------

echo
echo "========================================"
echo "Checking Wayland integration"
echo "========================================"

echo
echo "xkbcommon pkg-config requirements:"
pkg-config --print-requires xkbcommon || true

echo
echo "xkbcommon pkg-config file:"
cat "$SYSROOT/usr/lib/pkgconfig/xkbcommon.pc"


# ------------------------------------------------------------
# Done
# ------------------------------------------------------------

echo
echo "========================================"
echo "libxkbcommon Wayland build completed"
echo "========================================"
echo "Version         : $LIBXKBCOMMON_VER"
echo "Wayland support : ENABLED"
echo "X11 support     : DISABLED"
echo "Libraries       : STATIC + SHARED"
echo "========================================"