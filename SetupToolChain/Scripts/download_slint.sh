#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"
mkdir -p "$SOURCES"; cd "$SOURCES"
if [ ! -d "slint-$SLINT_VER" ]; then
    git clone --depth 1 --branch "v$SLINT_VER" https://github.com/slint-ui/slint.git "slint-$SLINT_VER"
fi
echo "Slint source: $SOURCES/slint-$SLINT_VER"