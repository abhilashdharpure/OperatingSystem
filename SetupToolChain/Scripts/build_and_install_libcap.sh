#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

SRC_DIR="$SOURCES/libcap-$LIBCAP_VER"

if [ ! -d "$SRC_DIR" ]; then
    echo "ERROR: libcap source directory not found:"
    echo "  $SRC_DIR"
    echo
    echo "Run:"
    echo "  $SCRIPT_DIR/download_libcap.sh"
    exit 1
fi

echo "========================================"
echo "Building libcap $LIBCAP_VER"
echo "========================================"
echo
echo "Source:"
echo "  $SRC_DIR"
echo
echo "Target:"
echo "  $TARGET"
echo
echo "Sysroot:"
echo "  $SYSROOT"
echo

# ============================================================
# Target toolchain
# ============================================================

export CROSS_COMPILE="$PREFIX/bin/$TARGET-"

export CC="${CROSS_COMPILE}gcc"
export AR="${CROSS_COMPILE}ar"
export RANLIB="${CROSS_COMPILE}ranlib"
export OBJCOPY="${CROSS_COMPILE}objcopy"

# ============================================================
# Build-host toolchain
#
# libcap generates helper programs such as _makenames.
# These must run on the Ubuntu build machine.
# ============================================================

export BUILD_CC="gcc"
export BUILD_LD="gcc"

# ============================================================
# Target compilation flags
#
# IMPORTANT:
# Do not override CPPFLAGS.
#
# libcap's Makefile adds its own required include paths through:
#
#   CPPFLAGS += $(LIBCAP_INCLUDES)
#
# which points to:
#
#   libcap/include/uapi
#   libcap/include
#
# ============================================================

export CFLAGS="-O2 -fPIC --sysroot=$SYSROOT"
export LDFLAGS="--sysroot=$SYSROOT"

# ============================================================
# Disable components that are not required by libudev/systemd
# ============================================================

export PAM_CAP=no
export GOLANG=no

echo "========================================"
echo "Checking target compiler"
echo "========================================"
echo

"$CC" --version

echo
echo "Target:"
"$CC" -dumpmachine

echo
echo "========================================"
echo "Checking build compiler"
echo "========================================"
echo

"$BUILD_CC" --version

echo
echo "Build machine:"
"$BUILD_CC" -dumpmachine

echo
echo "========================================"
echo "Cleaning previous libcap build"
echo "========================================"
echo

make \
    -C "$SRC_DIR" \
    CROSS_COMPILE="$CROSS_COMPILE" \
    BUILD_CC="$BUILD_CC" \
    BUILD_LD="$BUILD_LD" \
    PAM_CAP=no \
    GOLANG=no \
    clean

echo
echo "========================================"
echo "Building libcap"
echo "========================================"
echo

make \
    -C "$SRC_DIR" \
    CROSS_COMPILE="$CROSS_COMPILE" \
    CC="$CC" \
    AR="$AR" \
    RANLIB="$RANLIB" \
    OBJCOPY="$OBJCOPY" \
    BUILD_CC="$BUILD_CC" \
    BUILD_LD="$BUILD_LD" \
    PAM_CAP=no \
    GOLANG=no \
    DYNAMIC=yes \
    CFLAGS="$CFLAGS" \
    LDFLAGS="$LDFLAGS" \
    all

echo
echo "========================================"
echo "Installing libcap into target sysroot"
echo "========================================"
echo

make \
    -C "$SRC_DIR" \
    CROSS_COMPILE="$CROSS_COMPILE" \
    CC="$CC" \
    AR="$AR" \
    RANLIB="$RANLIB" \
    OBJCOPY="$OBJCOPY" \
    BUILD_CC="$BUILD_CC" \
    BUILD_LD="$BUILD_LD" \
    PAM_CAP=no \
    GOLANG=no \
    DYNAMIC=yes \
    prefix=/usr \
    lib=lib \
    DESTDIR="$SYSROOT" \
    CFLAGS="$CFLAGS" \
    LDFLAGS="$LDFLAGS" \
    install

echo
echo "========================================"
echo "Verifying libcap installation"
echo "========================================"
echo

echo "Capability header:"

if [ ! -f "$SYSROOT/usr/include/sys/capability.h" ]; then
    echo "ERROR: capability.h was not installed."
    exit 1
fi

ls -l "$SYSROOT/usr/include/sys/capability.h"

echo
echo "libcap libraries:"

find "$SYSROOT/usr/lib" \
    \( -name 'libcap.so*' -o -name 'libcap.a' -o -name 'libcap.pc' \) \
    -print | sort

echo
echo "========================================"
echo "Checking target ELF"
echo "========================================"
echo

if [ -f "$SYSROOT/usr/lib/libcap.so" ]; then
    file "$SYSROOT/usr/lib/libcap.so"
fi

if [ -f "$SYSROOT/usr/lib/libcap.a" ]; then
    file "$SYSROOT/usr/lib/libcap.a"
fi

echo
echo "========================================"
echo "libcap installation complete"
echo "========================================"
echo
echo "Installed header:"
echo "  $SYSROOT/usr/include/sys/capability.h"
echo
echo "Installed libraries:"
echo "  $SYSROOT/usr/lib/libcap.so"
echo "  $SYSROOT/usr/lib/libcap.a"
echo
echo "Next step:"
echo
echo "  $SCRIPT_DIR/configure_libudev.sh"
echo