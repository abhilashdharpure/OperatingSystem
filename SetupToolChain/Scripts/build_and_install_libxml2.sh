#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

# ------------------------------------------------------------
# Target pkg-config environment
# ------------------------------------------------------------

export PKG_CONFIG_SYSROOT_DIR="$SYSROOT"
export PKG_CONFIG_LIBDIR="$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/share/pkgconfig:$SYSROOT/usr/lib64/pkgconfig"
unset PKG_CONFIG_PATH


# ------------------------------------------------------------
# Paths
# ------------------------------------------------------------

LIBXML2_SRC="$SOURCES/libxml2-$LIBXML2_VER"
BUILD_DIR="$TOP/build-libxml2"


# ------------------------------------------------------------
# Information
# ------------------------------------------------------------

echo "========================================"
echo "Building libxml2 $LIBXML2_VER"
echo "========================================"
echo "Target       : $TARGET"
echo "Sysroot      : $SYSROOT"
echo "Prefix       : $PREFIX"
echo "Source       : $LIBXML2_SRC"
echo "Build        : $BUILD_DIR"
echo


# ------------------------------------------------------------
# Check source
# ------------------------------------------------------------

if [ ! -d "$LIBXML2_SRC" ]; then
    echo "ERROR: libxml2 source not found:"
    echo "$LIBXML2_SRC"
    echo
    echo "Run:"
    echo "  $SCRIPT_DIR/download_libxml2.sh"
    exit 1
fi


# ------------------------------------------------------------
# Check cross compiler
# ------------------------------------------------------------

if [ ! -x "$PREFIX/bin/$TARGET-gcc" ]; then
    echo "ERROR: Cross compiler not found:"
    echo "$PREFIX/bin/$TARGET-gcc"
    exit 1
fi


# ------------------------------------------------------------
# Check target sysroot
# ------------------------------------------------------------

if [ ! -f "$SYSROOT/usr/lib/libc.a" ]; then
    echo "ERROR: musl libc not found:"
    echo "$SYSROOT/usr/lib/libc.a"
    echo
    echo "Build/install musl before building libxml2."
    exit 1
fi


# ------------------------------------------------------------
# Check pkg-config
# ------------------------------------------------------------

if ! command -v pkg-config >/dev/null 2>&1; then
    echo "ERROR: pkg-config not found."
    exit 1
fi


# ------------------------------------------------------------
# Clean build directory
# ------------------------------------------------------------

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR"


# ------------------------------------------------------------
# Configure
# ------------------------------------------------------------

echo "========================================"
echo "Configuring libxml2"
echo "========================================"

CFLAGS="-fPIC" \
"$LIBXML2_SRC/configure" \
    --build="$(gcc -dumpmachine)" \
    --host="$TARGET" \
    --prefix=/usr \
    --disable-shared \
    --enable-static \
    --without-python \
    --without-zlib \
    --without-lzma \
    --without-readline \
    --without-debug \
    --without-ftp \
    --without-http

# ------------------------------------------------------------
# Build
# ------------------------------------------------------------

echo
echo "========================================"
echo "Building libxml2"
echo "========================================"

make -j"$(nproc)"


# ------------------------------------------------------------
# Install
# ------------------------------------------------------------

echo
echo "========================================"
echo "Installing libxml2 into target sysroot"
echo "========================================"

make DESTDIR="$SYSROOT" install


# ------------------------------------------------------------
# Verify installation
# ------------------------------------------------------------

echo
echo "========================================"
echo "Verifying libxml2 installation"
echo "========================================"

if [ ! -f "$SYSROOT/usr/include/libxml2/libxml/parser.h" ]; then
    echo "ERROR: libxml2 headers were not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/libxml2.a" ]; then
    echo "ERROR: libxml2 static library was not installed."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/pkgconfig/libxml-2.0.pc" ]; then
    echo "ERROR: libxml-2.0.pc was not installed."
    exit 1
fi


# ------------------------------------------------------------
# Verify pkg-config
# ------------------------------------------------------------

echo
echo "========================================"
echo "Checking target pkg-config"
echo "========================================"

echo "Version:"
pkg-config --modversion libxml-2.0

echo
echo "CFLAGS:"
pkg-config --cflags libxml-2.0

echo
echo "LIBS:"
pkg-config --libs libxml-2.0


# ------------------------------------------------------------
# Show installed files
# ------------------------------------------------------------

echo
echo "========================================"
echo "Installed libxml2 files"
echo "========================================"

find "$SYSROOT/usr" \
    \( \
        -name 'libxml2.a' \
        -o -name 'libxml-2.0.pc' \
        -o -name 'parser.h' \
        -o -name 'xmlversion.h' \
    \) \
    -print


# ------------------------------------------------------------
# Done
# ------------------------------------------------------------

echo
echo "========================================"
echo "libxml2 installation completed"
echo "========================================"