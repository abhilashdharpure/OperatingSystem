#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

mkdir -p "$SOURCES"
cd "$SOURCES"

echo "========================================"
echo "Downloading Linux kernel $LINUX_VER"
echo "========================================"

wget -nc \
    "https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-${LINUX_VER}.tar.xz"

if [ ! -d "$SOURCES/linux-$LINUX_VER" ]; then
    echo "Extracting Linux kernel..."
    tar -xf "linux-${LINUX_VER}.tar.xz"
else
    echo "Linux kernel source already extracted."
fi

echo
echo "Linux kernel source:"
ls -ld "$SOURCES/linux-$LINUX_VER"

echo
echo "Linux kernel download completed."