#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

SRC_DIR="$SOURCES/util-linux-$UTIL_LINUX_VER"

echo "========================================"
echo "Building util-linux $UTIL_LINUX_VER"
echo "========================================"

echo "SRC_DIR = $SRC_DIR"
echo "SYSROOT = $SYSROOT"
echo "PREFIX  = $PREFIX"
echo "TARGET  = $TARGET"

if [ ! -d "$SRC_DIR" ]; then
    echo "ERROR: util-linux source directory not found:"
    echo "  $SRC_DIR"
    echo
    echo "Run download_util_linux.sh first."
    exit 1
fi

export CC="$PREFIX/bin/$TARGET-gcc"
export AR="$PREFIX/bin/$TARGET-ar"
export RANLIB="$PREFIX/bin/$TARGET-ranlib"
export STRIP="$PREFIX/bin/$TARGET-strip"

export CFLAGS="-O2 -fPIC --sysroot=$SYSROOT"
export CXXFLAGS="-O2 -fPIC --sysroot=$SYSROOT"
export LDFLAGS="--sysroot=$SYSROOT"

export PKG_CONFIG_SYSROOT_DIR="$SYSROOT"
export PKG_CONFIG_LIBDIR="$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/share/pkgconfig:$SYSROOT/usr/lib64/pkgconfig"
unset PKG_CONFIG_PATH

echo
echo "CC:"
echo "  $CC"

echo
echo "Cleaning previous util-linux configuration..."
make -C "$SRC_DIR" distclean >/dev/null 2>&1 || true

echo
echo "Configuring util-linux..."

cd "$SRC_DIR"

./configure \
    --host="$TARGET" \
    --prefix=/usr \
    --libdir=/usr/lib \
    --disable-all-programs \
    --enable-libmount \
    --enable-libblkid \
    --enable-libuuid \
    --disable-static \
    --disable-nls \
    --without-python \
    --without-systemd \
    --without-tinfo \
    --without-readline \
    --without-ncurses \
    --without-ncursesw

echo
echo "========================================"
echo "Building util-linux libraries"
echo "========================================"

make -j"$(nproc)" \
    libmount.la \
    libblkid.la \
    libuuid.la

echo
echo "========================================"
echo "Installing into target sysroot"
echo "========================================"

make install \
    DESTDIR="$SYSROOT"

echo
echo "========================================"
echo "Verifying util-linux installation"
echo "========================================"

echo
echo "Libraries:"
find "$SYSROOT/usr/lib" \
    -maxdepth 2 \
    -type f \
    \( \
        -name 'libmount*' \
        -o -name 'libblkid*' \
        -o -name 'libuuid*' \
    \) \
    -print

echo
echo "pkg-config files:"
find "$SYSROOT/usr" \
    -type f \
    \( \
        -name 'mount.pc' \
        -o -name 'blkid.pc' \
        -o -name 'uuid.pc' \
    \) \
    -print

echo
echo "========================================"
echo "Testing pkg-config"
echo "========================================"

if pkg-config --exists mount; then
    echo "OK: mount.pc found"
    echo "Version: $(pkg-config --modversion mount)"
    echo "CFLAGS:  $(pkg-config --cflags mount)"
    echo "LIBS:    $(pkg-config --libs mount)"
else
    echo "ERROR: pkg-config cannot find mount"
    exit 1
fi

echo
echo "util-linux libraries installed successfully."