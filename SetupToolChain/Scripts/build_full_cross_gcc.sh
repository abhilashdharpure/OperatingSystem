#!/bin/bash
set -e

# Always resolve paths relative to SetupToolChain
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR/.."

source "./Scripts/env.sh"

echo "============================================================"
echo "Building full cross GCC $GCC_VER"
echo "============================================================"

echo
echo "Target  : $TARGET"
echo "Prefix  : $PREFIX"
echo "Sysroot : $SYSROOT"

echo
echo "============================================================"
echo "Verifying stage-1 GCC"
echo "============================================================"

"$PREFIX/bin/$TARGET-gcc" --version

echo
echo "Checking libgcc..."

LIBGCC_PATH="$("$PREFIX/bin/$TARGET-gcc" -print-libgcc-file-name)"

echo "libgcc:"
echo "$LIBGCC_PATH"

if [ ! -f "$LIBGCC_PATH" ]; then
    echo "ERROR: libgcc.a not found."
    exit 1
fi

echo
echo "============================================================"
echo "Verifying musl"
echo "============================================================"

if [ ! -f "$SYSROOT/usr/lib/libc.so" ]; then
    echo "ERROR: musl libc.so not found."
    exit 1
fi

if [ ! -f "$SYSROOT/usr/lib/crt1.o" ]; then
    echo "ERROR: musl crt1.o not found."
    exit 1
fi

echo "Found musl libc and startup files."

echo
echo "============================================================"
echo "Cleaning previous GCC stage-2 build"
echo "============================================================"

rm -rf "$TOP/build-gcc-stage2"

mkdir -p "$TOP/build-gcc-stage2"
cd "$TOP/build-gcc-stage2"

echo
echo "============================================================"
echo "Configuring full GCC"
echo "============================================================"

"$SOURCES/gcc-$GCC_VER/configure" \
    --prefix="$PREFIX" \
    --target="$TARGET" \
    --with-sysroot="$SYSROOT" \
    --enable-languages=c,c++ \
    --disable-multilib \
    --disable-bootstrap \
    --disable-shared \
    --disable-libsanitizer \
    --disable-libssp \
    --disable-libquadmath \
    --disable-libvtv \
    --disable-libgomp \
    --enable-threads=posix

echo
echo "============================================================"
echo "Building GCC compiler"
echo "============================================================"

make -j"$(nproc)" all-gcc

echo
echo "============================================================"
echo "Installing GCC compiler"
echo "============================================================"

make install-gcc

echo
echo "============================================================"
echo "Building target libgcc"
echo "============================================================"

make -j"$(nproc)" all-target-libgcc

echo
echo "============================================================"
echo "Installing target libgcc"
echo "============================================================"

make install-target-libgcc

echo
echo "============================================================"
echo "Building libstdc++"
echo "============================================================"

make -j"$(nproc)" all-target-libstdc++-v3

echo
echo "============================================================"
echo "Installing libstdc++"
echo "============================================================"

make install-target-libstdc++-v3

echo
echo "============================================================"
echo "Verifying GCC"
echo "============================================================"

"$PREFIX/bin/$TARGET-gcc" --version

echo
echo "------------------------------------------------------------"
"$PREFIX/bin/$TARGET-g++" --version

echo
echo "============================================================"
echo "Testing C compilation"
echo "============================================================"

cat > /tmp/test-gcc.c <<'EOF'
#include <stdio.h>

int main(void)
{
    printf("Hello from C\n");
    return 0;
}
EOF

"$PREFIX/bin/$TARGET-gcc" \
    --sysroot="$SYSROOT" \
    -o /tmp/test-gcc \
    /tmp/test-gcc.c

file /tmp/test-gcc

echo
echo "============================================================"
echo "Testing C++ compilation"
echo "============================================================"

cat > /tmp/test-g++.cpp <<'EOF'
#include <iostream>

int main()
{
    std::cout << "Hello from C++" << std::endl;
    return 0;
}
EOF

"$PREFIX/bin/$TARGET-g++" \
    --sysroot="$SYSROOT" \
    -o /tmp/test-g++ \
    /tmp/test-g++.cpp

file /tmp/test-g++

echo
echo "============================================================"
echo "Full cross GCC completed successfully"
echo "============================================================"

echo "Target  : $TARGET"
echo "GCC     : $GCC_VER"
echo "Sysroot : $SYSROOT"
echo "Prefix  : $PREFIX"

echo
echo "C compiler:"
echo "$PREFIX/bin/$TARGET-gcc"

echo
echo "C++ compiler:"
echo "$PREFIX/bin/$TARGET-g++"

echo
echo "libstdc++:"
find "$PREFIX/$TARGET" "$PREFIX/lib/gcc/$TARGET/$GCC_VER" \
    -type f \( -name 'libstdc++.a' -o -name 'libsupc++.a' \) \
    -print 2>/dev/null | sort || true
