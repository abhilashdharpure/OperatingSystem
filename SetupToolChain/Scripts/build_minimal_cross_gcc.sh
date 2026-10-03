#!/bin/bash
set -e

# Always resolve paths relative to SetupToolChain.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR/.."

source "./Scripts/env.sh"

echo "============================================================"
echo "Preparing musl headers"
echo "============================================================"

cd "$SOURCES/musl-$MUSL_VER"

# Configure musl only for generating/installing headers.

./configure 
--target="$TARGET" 
--prefix=/usr

make install-headers DESTDIR="$SYSROOT"

echo "============================================================"
echo "Cleaning previous GCC stage-1 build"
echo "============================================================"

rm -rf "$TOP/build-gcc-stage1"

mkdir -p "$TOP/build-gcc-stage1"
cd "$TOP/build-gcc-stage1"

echo "============================================================"
echo "Configuring GCC stage 1"
echo "============================================================"

"$SOURCES/gcc-$GCC_VER/configure" 
--prefix="$PREFIX" 
--target="$TARGET" 
--with-sysroot="$SYSROOT" 
--enable-languages=c 
--disable-multilib 
--disable-bootstrap 
--disable-libsanitizer 
--without-headers 
--disable-shared 
--disable-libssp 
--disable-libquadmath 
--disable-libvtv 
--disable-libgomp

echo "============================================================"
echo "Building GCC compiler"
echo "============================================================"

make all-gcc -j"$(nproc)"

echo
echo "Installing GCC compiler..."
make install-gcc

echo "============================================================"
echo "Building static libgcc"
echo "============================================================"

make all-target-libgcc -j"$(nproc)"

echo "============================================================"
echo "Installing static libgcc"
echo "============================================================"

make install-target-libgcc

echo "============================================================"
echo "Verifying stage-1 GCC"
echo "============================================================"

"$PREFIX/bin/$TARGET-gcc" --version

echo
echo "============================================================"
echo "Verifying static libgcc"
echo "============================================================"

LIBGCC_PATH="$("$PREFIX/bin/$TARGET-gcc" -print-libgcc-file-name)"

echo "libgcc path:"
echo "$LIBGCC_PATH"

if [ ! -f "$LIBGCC_PATH" ]; then
echo
echo "ERROR: libgcc.a was not installed correctly."
exit 1
fi

echo
echo "Installed GCC runtime libraries:"

find "$PREFIX/lib/gcc/$TARGET/$GCC_VER" 
-maxdepth 1 
-type f 
\(-name 'libgcc*.a' -o -name 'libgcov.a'\) 
-print | sort

echo
echo "============================================================"
echo "Verifying libgcc contains required runtime symbols"
echo "============================================================"

echo "Checking for complex arithmetic runtime functions..."

if command -v nm >/dev/null 2>&1; then
nm "$LIBGCC_PATH" | grep -E '__mul(dc|sc|xc)3' || true
fi

echo
echo "============================================================"
echo "GCC stage-1 + static libgcc completed successfully"
echo "============================================================"

echo "Target  : $TARGET"
echo "GCC     : $GCC_VER"
echo "Sysroot : $SYSROOT"
echo "Prefix  : $PREFIX"

echo
echo "Static libgcc:"
echo "$LIBGCC_PATH"

echo
echo "Next step: build and install musl."
