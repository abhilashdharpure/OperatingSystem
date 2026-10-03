#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

mkdir -p "$SOURCES"
cd "$SOURCES"

echo "========================================"
echo "Downloading Expat $EXPAT_VER"
echo "========================================"

wget -nc \
    "https://github.com/libexpat/libexpat/releases/download/R_${EXPAT_VER//./_}/expat-${EXPAT_VER}.tar.xz"

if [ ! -d "$SOURCES/expat-$EXPAT_VER" ]; then
    echo "Extracting Expat..."
    tar -xf "expat-${EXPAT_VER}.tar.xz"
else
    echo "Expat-$EXPAT_VER already extracted."
fi

echo
echo "Expat source:"
ls -ld "$SOURCES/expat-$EXPAT_VER"

echo
echo "Expat download completed."