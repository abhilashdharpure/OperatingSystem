#!/bin/bash
set -e

# Always resolve paths relative to SetupToolChain
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR/.."

source "./Scripts/env.sh"

echo "============================================================"
echo "Building musl $MUSL_VER"
echo "============================================================"

rm -rf "$TOP/build-musl"
mkdir -p "$TOP/build-musl"
cd "$TOP/build-musl"

# Use the stage-1 cross compiler
export CC="$PREFIX/bin/$TARGET-gcc"

echo "Using compiler:"
"$CC" --version

echo
echo "Using target:"
echo "$TARGET"

echo
echo "Checking static libgcc..."

LIBGCC_PATH="$("$CC" -print-libgcc-file-name)"

echo "libgcc:"
echo "$LIBGCC_PATH"

if [ ! -f "$LIBGCC_PATH" ]; then
    echo
    echo "ERROR: static libgcc.a was not found."
    exit 1
fi

echo
echo "============================================================"
echo "Configuring musl"
echo "============================================================"

"$SOURCES/musl-$MUSL_VER/configure" \
    --target="$TARGET" \
    --prefix=/usr \
    --syslibdir=/lib

echo
echo "============================================================"
echo "Building musl"
echo "============================================================"

make -j"$(nproc)"

echo
echo "============================================================"
echo "Installing musl into sysroot"
echo "============================================================"

make DESTDIR="$SYSROOT" install

echo
echo "============================================================"
echo "Verifying musl installation"
echo "============================================================"

echo "Sysroot:"
echo "$SYSROOT"

echo
echo "Installed musl files:"

find "$SYSROOT" -maxdepth 4 -type f | sort | head -100

echo
echo "============================================================"
echo "Checking libc"
echo "============================================================"

if [ -f "$SYSROOT/usr/lib/libc.so" ]; then
    echo "Found: $SYSROOT/usr/lib/libc.so"
else
    echo "ERROR: libc.so was not found."
    exit 1
fi

echo
echo "============================================================"
echo "musl installation completed successfully."
echo "============================================================"

echo "Target  : $TARGET"
echo "musl    : $MUSL_VER"
echo "Sysroot : $SYSROOT"
echo "libgcc  : $LIBGCC_PATH"

