#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

BUILD_DIR="$TOP/build-systemd-libudev"

export PKG_CONFIG_SYSROOT_DIR="$SYSROOT"
export PKG_CONFIG_LIBDIR="$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/share/pkgconfig:$SYSROOT/usr/lib64/pkgconfig"
unset PKG_CONFIG_PATH

echo "========================================"
echo "Configure systemd with musl printf"
echo "========================================"
echo
echo "Build dir : $BUILD_DIR"
echo "Sysroot   : $SYSROOT"
echo "Library   : $SYSROOT/usr/lib/libmuslprintf.a"
echo

# ------------------------------------------------------------

# Verify compatibility library

# ------------------------------------------------------------

if [ ! -f "$SYSROOT/usr/lib/libmuslprintf.a" ]; then
echo "ERROR: libmuslprintf.a not found:"
echo "  $SYSROOT/usr/lib/libmuslprintf.a"
echo
echo "Run create_musl_printf_parser.sh first."
exit 1
fi

# ------------------------------------------------------------

# Verify printf.h

# ------------------------------------------------------------

if [ ! -f "$SYSROOT/usr/include/printf.h" ]; then
echo "ERROR: printf.h not found:"
echo "  $SYSROOT/usr/include/printf.h"
echo
echo "Run create_musl_printf_h.sh first."
exit 1
fi

# ------------------------------------------------------------

# Verify existing Meson build

# ------------------------------------------------------------

if [ ! -d "$BUILD_DIR" ]; then
echo "ERROR: systemd Meson build directory does not exist:"
echo "  $BUILD_DIR"
echo
echo "Run configure_libudev.sh first."
exit 1
fi

if [ ! -f "$BUILD_DIR/build.ninja" ]; then
echo "ERROR: Meson build.ninja does not exist:"
echo "  $BUILD_DIR/build.ninja"
echo
echo "Run configure_libudev.sh first."
exit 1
fi

# ------------------------------------------------------------

# Configure from inside the existing Meson build directory

# ------------------------------------------------------------

echo "Adding musl printf compatibility library..."
echo

(
cd "$BUILD_DIR"

```
meson configure \
    -Dc_link_args=-lmuslprintf
```

)

echo
echo "========================================"
echo "SUCCESS"
echo "========================================"
echo
echo "Meson configured with:"
echo
echo "  -lmuslprintf"
echo
echo "Next step:"
echo
echo "  ./Scripts/build_and_install_udev.sh"
echo
