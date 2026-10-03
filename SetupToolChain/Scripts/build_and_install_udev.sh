#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

SRC_DIR="$SOURCES/systemd-$SYSTEMD_VER"
BUILD_DIR="$TOP/build-systemd-libudev"

export PKG_CONFIG_SYSROOT_DIR="$SYSROOT"
export PKG_CONFIG_LIBDIR="$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/share/pkgconfig:$SYSROOT/usr/lib64/pkgconfig"
unset PKG_CONFIG_PATH

echo "========================================"
echo "Building systemd libudev $SYSTEMD_VER"
echo "========================================"

echo "Source      : $SRC_DIR"
echo "Build       : $BUILD_DIR"
echo "Sysroot     : $SYSROOT"
echo "Target      : $TARGET"

if [ ! -d "$SRC_DIR" ]; then
    echo "ERROR: systemd source directory not found:"
    echo "  $SRC_DIR"
    exit 1
fi

if [ ! -d "$BUILD_DIR" ]; then
    echo "ERROR: systemd libudev build directory not found:"
    echo "  $BUILD_DIR"
    echo
    echo "Run configure_libudev.sh first."
    exit 1
fi

if [ ! -f "$BUILD_DIR/build.ninja" ]; then
    echo "ERROR: Meson build configuration not found:"
    echo "  $BUILD_DIR/build.ninja"
    echo
    echo "Run configure_libudev.sh first."
    exit 1
fi

echo
echo "========================================"
echo "Building libudev"
echo "========================================"

ninja -C "$BUILD_DIR" -j"$(nproc)"

echo
echo "========================================"
echo "Installing libudev into target sysroot"
echo "========================================"

DESTDIR="$SYSROOT" ninja -C "$BUILD_DIR" install

echo
echo "========================================"
echo "Verifying libudev installation"
echo "========================================"

echo
echo "libudev libraries:"
find "$SYSROOT/usr/lib" \
    -maxdepth 2 \
    -type f \
    \( \
        -name 'libudev.so*' \
        -o -name 'libudev.la' \
    \) \
    -print

echo
echo "libudev headers:"
find "$SYSROOT/usr/include" \
    -type f \
    -path '*/libudev.h' \
    -print

echo
echo "libudev pkg-config:"
find "$SYSROOT/usr" \
    -type f \
    -name 'libudev.pc' \
    -print

echo
echo "========================================"
echo "Testing libudev pkg-config"
echo "========================================"

if pkg-config --exists libudev; then
    echo "OK: libudev.pc found"
    echo "Version: $(pkg-config --modversion libudev)"
    echo "CFLAGS:  $(pkg-config --cflags libudev)"
    echo "LIBS:    $(pkg-config --libs libudev)"
else
    echo "ERROR: pkg-config cannot find libudev"
    exit 1
fi

echo
echo "========================================"
echo "Verifying target library"
echo "========================================"

LIBUDEV="$SYSROOT/usr/lib/libudev.so"

if [ -f "$LIBUDEV" ]; then
    echo "OK: $LIBUDEV"
else
    echo "ERROR: libudev.so was not installed:"
    echo "  $LIBUDEV"
    exit 1
fi

echo
echo "========================================"
echo "libudev installation successful"
echo "========================================"