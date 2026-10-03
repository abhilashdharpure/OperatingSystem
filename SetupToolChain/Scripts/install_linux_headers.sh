#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

LINUX_SRC="$SOURCES/linux-$LINUX_VER"

echo "========================================"
echo "Installing Linux kernel UAPI headers"
echo "========================================"
echo "Kernel  : $LINUX_VER"
echo "Sysroot : $SYSROOT"
echo

if [ ! -d "$LINUX_SRC" ]; then
    echo "ERROR: Linux source not found:"
    echo "$LINUX_SRC"
    echo
    echo "Run:"
    echo "  ./Scripts/download_linux_headers.sh"
    exit 1
fi

make -C "$LINUX_SRC" \
    headers_install \
    ARCH=x86_64 \
    INSTALL_HDR_PATH="$SYSROOT/usr"

echo
echo "========================================"
echo "Kernel headers installed"
echo "========================================"

echo
echo "Checking linux/limits.h..."

if [ -f "$SYSROOT/usr/include/linux/limits.h" ]; then
    echo "OK: $SYSROOT/usr/include/linux/limits.h"
else
    echo "ERROR: linux/limits.h was not installed."
    exit 1
fi