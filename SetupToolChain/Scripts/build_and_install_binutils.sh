#!/bin/bash
set -e

source ./Scripts/env.sh

echo "Building Binutils..."

# Clean build directory if re-running
rm -rf "$TOP/binutils"

mkdir -p "$TOP/binutils"
cd "$TOP/binutils"

"$SOURCES/binutils-$BINUTILS_VER/configure" \
    --prefix="$PREFIX" \
    --target="$TARGET" \
    --disable-multilib \
    --disable-gprofng \
    --with-sysroot="$SYSROOT"

make -j$(nproc)

make install

export PATH="$PREFIX/bin:$PATH"

echo "Binutils installation completed."