#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

LIBFFI_SRC="$SOURCES/libffi-$LIBFFI_VER"
BUILD_DIR="$TOP/build-libffi"

echo "========================================"
echo "Building libffi $LIBFFI_VER"
echo "========================================"

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

CFLAGS="-fPIC" \
CXXFLAGS="-fPIC" \
"$LIBFFI_SRC/configure" \
    --build="$(gcc -dumpmachine)" \
    --host="$TARGET" \
    --prefix=/usr \
    --disable-shared \
    --enable-static \
    --disable-docs

make -j"$(nproc)"

make DESTDIR="$SYSROOT" install

echo
echo "========================================"
echo "Verifying libffi"
echo "========================================"

if [ ! -f "$SYSROOT/usr/lib64/libffi.a" ]; then
    echo "ERROR: libffi.a was not installed:"
    echo "$SYSROOT/usr/lib64/libffi.a"
    exit 1
fi

echo "libffi installed:"
ls -lh "$SYSROOT/usr/lib64/libffi.a"

echo
echo "libffi build completed successfully."