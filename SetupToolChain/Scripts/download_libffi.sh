#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

mkdir -p "$SOURCES"
cd "$SOURCES"

echo "========================================"
echo "Downloading libffi $LIBFFI_VER"
echo "========================================"

wget -nc \
    "https://github.com/libffi/libffi/releases/download/v${LIBFFI_VER}/libffi-${LIBFFI_VER}.tar.gz"

if [ ! -d "$SOURCES/libffi-$LIBFFI_VER" ]; then
    echo "Extracting libffi..."
    tar -xf "libffi-$LIBFFI_VER.tar.gz"
else
    echo "libffi-$LIBFFI_VER already extracted."
fi

echo
echo "libffi source:"
ls -ld "$SOURCES/libffi-$LIBFFI_VER"

echo
echo "libffi download completed."