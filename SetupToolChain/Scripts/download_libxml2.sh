#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

mkdir -p "$SOURCES"
cd "$SOURCES"

echo "========================================"
echo "Downloading libxml2 $LIBXML2_VER"
echo "========================================"

LIBXML2_ARCHIVE="libxml2-$LIBXML2_VER.tar.xz"
LIBXML2_SRC="$SOURCES/libxml2-$LIBXML2_VER"

wget -nc \
    "https://download.gnome.org/sources/libxml2/2.14/$LIBXML2_ARCHIVE"

if [ ! -d "$LIBXML2_SRC" ]; then
    echo "Extracting libxml2..."
    tar -xf "$LIBXML2_ARCHIVE"
else
    echo "libxml2-$LIBXML2_VER already extracted."
fi

echo
echo "libxml2 source:"
ls -ld "$LIBXML2_SRC"

echo
echo "libxml2 download completed."