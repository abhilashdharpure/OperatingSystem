#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

# ============================================================
# Paths
# ============================================================

WAYLAND_SRC="$SOURCES/wayland-$WAYLAND_VER"

TARGET_BUILD_DIR="$TOP/build-wayland"
NATIVE_BUILD_DIR="$TOP/build-wayland-native"

# Native tools used while cross-compiling Wayland
NATIVE_PREFIX="$PREFIX/native"

# ============================================================
# Target pkg-config environment
# ============================================================

TARGET_PKG_CONFIG_SYSROOT_DIR="$SYSROOT"
TARGET_PKG_CONFIG_LIBDIR="$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/share/pkgconfig:$SYSROOT/usr/lib64/pkgconfig"

echo "========================================"
echo "Building Wayland $WAYLAND_VER"
echo "========================================"
echo "Target              : $TARGET"
echo "Sysroot             : $SYSROOT"
echo "Prefix              : $PREFIX"
echo "Source              : $WAYLAND_SRC"
echo "Target build        : $TARGET_BUILD_DIR"
echo "Native build        : $NATIVE_BUILD_DIR"
echo "Native prefix       : $NATIVE_PREFIX"
echo

# ============================================================
# Checks
# ============================================================

if [ ! -d "$WAYLAND_SRC" ]; then
    echo "ERROR: Wayland source not found:"
    echo "$WAYLAND_SRC"
    echo
    echo "Run:"
    echo "  $SCRIPT_DIR/download_wayland.sh"
    exit 1
fi

if [ ! -x "$PREFIX/bin/$TARGET-gcc" ]; then
    echo "ERROR: Cross compiler not found:"
    echo "$PREFIX/bin/$TARGET-gcc"
    exit 1
fi

if [ ! -x "$PREFIX/bin/$TARGET-g++" ]; then
    echo "ERROR: Cross C++ compiler not found:"
    echo "$PREFIX/bin/$TARGET-g++"
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib64/libffi.a" ]; then
    echo "ERROR: libffi not found:"
    echo "$SYSROOT/usr/lib64/libffi.a"
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/libexpat.a" ]; then
    echo "ERROR: Expat not found:"
    echo "$SYSROOT/usr/lib/libexpat.a"
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/libxml2.a" ]; then
    echo "ERROR: libxml2 not found:"
    echo "$SYSROOT/usr/lib/libxml2.a"
    exit 1
fi

if [ ! -f "$MESON_CROSS_FILE" ]; then
    echo "ERROR: Meson cross file not found:"
    echo "$MESON_CROSS_FILE"
    exit 1
fi

if ! command -v meson >/dev/null 2>&1; then
    echo "ERROR: Meson is not installed."
    exit 1
fi

if ! command -v ninja >/dev/null 2>&1; then
    echo "ERROR: Ninja is not installed."
    exit 1
fi

# ============================================================
# Stage 1:
# Build native Wayland scanner
#
# IMPORTANT:
# The native scanner MUST use Ubuntu's native dependencies.
# It must NOT see the target sysroot through pkg-config.
# ============================================================

echo
echo "========================================"
echo "Stage 1: Building native Wayland scanner"
echo "========================================"
echo

rm -rf "$NATIVE_BUILD_DIR"
mkdir -p "$NATIVE_BUILD_DIR"

# Save current pkg-config environment.
SAVED_PKG_CONFIG_SYSROOT_DIR="${PKG_CONFIG_SYSROOT_DIR-}"
SAVED_PKG_CONFIG_LIBDIR="${PKG_CONFIG_LIBDIR-}"
SAVED_PKG_CONFIG_PATH="${PKG_CONFIG_PATH-}"

# Completely remove target pkg-config environment.
unset PKG_CONFIG_SYSROOT_DIR
unset PKG_CONFIG_LIBDIR
unset PKG_CONFIG_PATH

echo "Native pkg-config environment:"
echo "  PKG_CONFIG_SYSROOT_DIR = ${PKG_CONFIG_SYSROOT_DIR-<unset>}"
echo "  PKG_CONFIG_LIBDIR      = ${PKG_CONFIG_LIBDIR-<unset>}"
echo "  PKG_CONFIG_PATH        = ${PKG_CONFIG_PATH-<unset>}"
echo

meson setup "$NATIVE_BUILD_DIR" \
    "$WAYLAND_SRC" \
    --prefix="$NATIVE_PREFIX" \
    --libdir=lib \
    --buildtype=release \
    -Ddocumentation=false \
    -Ddtd_validation=true \
    -Dlibraries=false \
    -Dtests=false

echo
echo "========================================"
echo "Building native Wayland scanner"
echo "========================================"

ninja -C "$NATIVE_BUILD_DIR"

echo
echo "========================================"
echo "Installing native Wayland scanner"
echo "========================================"

ninja -C "$NATIVE_BUILD_DIR" install

if [ ! -x "$NATIVE_PREFIX/bin/wayland-scanner" ]; then
    echo "ERROR: Native wayland-scanner was not installed:"
    echo "$NATIVE_PREFIX/bin/wayland-scanner"
    exit 1
fi

echo
echo "Native scanner:"
"$NATIVE_PREFIX/bin/wayland-scanner" --version || true
if [ ! -f "$NATIVE_PREFIX/lib/pkgconfig/wayland-scanner.pc" ]; then
    echo "ERROR: Native wayland-scanner.pc was not installed:"
    echo "$NATIVE_PREFIX/lib/pkgconfig/wayland-scanner.pc"
    exit 1
fi

# ============================================================
# Stage 2:
# Cross-build target Wayland
# ============================================================

echo
echo "========================================"
echo "Stage 2: Cross-building target Wayland"
echo "========================================"
echo

# Restore target pkg-config environment.
if [ -n "$SAVED_PKG_CONFIG_SYSROOT_DIR" ]; then
    export PKG_CONFIG_SYSROOT_DIR="$SAVED_PKG_CONFIG_SYSROOT_DIR"
else
    export PKG_CONFIG_SYSROOT_DIR="$TARGET_PKG_CONFIG_SYSROOT_DIR"
fi

if [ -n "$SAVED_PKG_CONFIG_LIBDIR" ]; then
    export PKG_CONFIG_LIBDIR="$SAVED_PKG_CONFIG_LIBDIR"
else
    export PKG_CONFIG_LIBDIR="$TARGET_PKG_CONFIG_LIBDIR"
fi

# Native Wayland scanner .pc file.
#
# Meson uses:
#
#   dependency('wayland-scanner', native: true)
#
# so this must point to the native installation.
export PKG_CONFIG_PATH="$NATIVE_PREFIX/lib/pkgconfig:$NATIVE_PREFIX/share/pkgconfig"

echo "Target pkg-config environment:"
echo "  PKG_CONFIG_SYSROOT_DIR = $PKG_CONFIG_SYSROOT_DIR"
echo "  PKG_CONFIG_LIBDIR      = $PKG_CONFIG_LIBDIR"
echo "  PKG_CONFIG_PATH        = $PKG_CONFIG_PATH"
echo

rm -rf "$TARGET_BUILD_DIR"
mkdir -p "$TARGET_BUILD_DIR"

meson setup "$TARGET_BUILD_DIR" \
    "$WAYLAND_SRC" \
    --cross-file "$MESON_CROSS_FILE" \
    --native-file "$SCRIPT_DIR/meson/native-wayland.ini" \
    --prefix=/usr \
    --buildtype=release \
    -Ddefault_library=both \
    -Ddocumentation=false \
    -Ddtd_validation=true \
    -Dlibraries=true \
    -Dscanner=true \
    -Dtests=false

echo
echo "========================================"
echo "Building target Wayland"
echo "========================================"

ninja -C "$TARGET_BUILD_DIR"

echo
echo "========================================"
echo "Installing target Wayland"
echo "========================================"

DESTDIR="$SYSROOT" ninja -C "$TARGET_BUILD_DIR" install

# ============================================================
# Verification
# ============================================================

echo
echo "========================================"
echo "Verifying target Wayland"
echo "========================================"

if [ ! -f "$SYSROOT/usr/lib/libwayland-client.a" ]; then
    echo "ERROR: libwayland-client.a was not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/libwayland-client.so" ]; then
    echo "ERROR: libwayland-client.so was not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/libwayland-server.a" ]; then
    echo "ERROR: libwayland-server.a was not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/libwayland-server.so" ]; then
    echo "ERROR: libwayland-server.so was not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/libwayland-cursor.a" ]; then
    echo "ERROR: libwayland-cursor.a was not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/libwayland-cursor.so" ]; then
    echo "ERROR: libwayland-cursor.so was not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/libwayland-egl.a" ]; then
    echo "ERROR: libwayland-egl.a was not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/libwayland-egl.so" ]; then
    echo "ERROR: libwayland-egl.so was not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/bin/wayland-scanner" ]; then
    echo "ERROR: target wayland-scanner was not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/pkgconfig/wayland-client.pc" ]; then
    echo "ERROR: wayland-client.pc was not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/pkgconfig/wayland-server.pc" ]; then
    echo "ERROR: wayland-server.pc was not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/pkgconfig/wayland-scanner.pc" ]; then
    echo "ERROR: wayland-scanner.pc was not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/include/wayland-client.h" ]; then
    echo "ERROR: Wayland client headers were not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/include/wayland-server-core.h" ]; then
    echo "ERROR: Wayland server headers were not installed."
    exit 1
fi

echo
echo "========================================"
echo "Checking target pkg-config"
echo "========================================"

echo
echo "wayland-client:"
pkg-config --modversion wayland-client
pkg-config --cflags wayland-client
pkg-config --libs wayland-client

echo
echo "wayland-server:"
pkg-config --modversion wayland-server
pkg-config --cflags wayland-server
pkg-config --libs wayland-server

echo
echo "wayland-scanner:"
pkg-config --modversion wayland-scanner
pkg-config --cflags wayland-scanner
pkg-config --libs wayland-scanner

echo
echo "========================================"
echo "Installed Wayland files"
echo "========================================"

find "$SYSROOT/usr" \
    \( \
        -name 'libwayland-*.a' \
        -o -name 'libwayland-*.so*' \
        -o -name 'wayland-*.pc' \
        -o -name 'wayland-scanner' \
    \) \
    -print

echo
echo "========================================"
echo "Wayland installation completed"
echo "========================================"
