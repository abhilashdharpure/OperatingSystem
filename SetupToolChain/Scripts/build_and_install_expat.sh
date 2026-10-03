#!/bin/bash
set -e

export PKG_CONFIG_SYSROOT_DIR="$SYSROOT"
export PKG_CONFIG_LIBDIR="$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/share/pkgconfig:$SYSROOT/usr/lib64/pkgconfig"
unset PKG_CONFIG_PATH


SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

EXPAT_SRC="$SOURCES/expat-$EXPAT_VER"
BUILD_DIR="$TOP/build-expat"

echo "========================================"
echo "Building Expat $EXPAT_VER"
echo "========================================"
echo "Target  : $TARGET"
echo "Sysroot : $SYSROOT"
echo "Source  : $EXPAT_SRC"
echo

if [ ! -d "$EXPAT_SRC" ]; then
    echo "ERROR: Expat source not found:"
    echo "$EXPAT_SRC"
    exit 1
fi

if [ ! -x "$PREFIX/bin/$TARGET-gcc" ]; then
    echo "ERROR: Cross compiler not found:"
    echo "$PREFIX/bin/$TARGET-gcc"
    exit 1
fi

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR"

echo "Configuring Expat..."

"$EXPAT_SRC/configure" \
    --build="$(gcc -dumpmachine)" \
    --host="$TARGET" \
    --prefix=/usr \
    --disable-shared \
    --enable-static \
    --without-xmlwf \
    --without-docbook

echo
echo "Building Expat..."

make -j"$(nproc)"

echo
echo "Installing Expat into target sysroot..."

make DESTDIR="$SYSROOT" install

echo
echo "========================================"
echo "Expat installation completed"
echo "========================================"

echo
echo "Installed Expat files:"

find "$SYSROOT/usr" \
    \( -name 'libexpat*.a' -o -name 'expat.h' -o -name 'expat_external.h' -o -name 'expat.pc' \) \
    -print