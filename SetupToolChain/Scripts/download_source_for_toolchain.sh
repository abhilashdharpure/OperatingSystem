#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

echo "========================================"
echo "Creating workspace directories"
echo "========================================"

mkdir -p "$SOURCES"
mkdir -p "$TOP"
mkdir -p "$PREFIX"
mkdir -p "$SYSROOT"
mkdir -p "$LOGS"

echo


# ============================================================
# Download bootstrap toolchain sources
# ============================================================

cd "$SOURCES"

echo "========================================"
echo "Downloading bootstrap toolchain sources"
echo "========================================"

wget -nc \
    "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VER.tar.xz"

wget -nc \
    "https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VER/gcc-$GCC_VER.tar.xz"

wget -nc \
    "https://ftp.gnu.org/gnu/gmp/gmp-$GMP_VER.tar.xz"

wget -nc \
    "https://ftp.gnu.org/gnu/mpfr/mpfr-$MPFR_VER.tar.xz"

wget -nc \
    "https://ftp.gnu.org/gnu/mpc/mpc-$MPC_VER.tar.gz"

wget -nc \
    "https://musl.libc.org/releases/musl-$MUSL_VER.tar.gz"


# ============================================================
# Extract bootstrap toolchain sources
# ============================================================

echo
echo "========================================"
echo "Extracting bootstrap toolchain sources"
echo "========================================"

if [ ! -d "$SOURCES/binutils-$BINUTILS_VER" ]; then
    echo "Extracting Binutils..."
    tar -xf "binutils-$BINUTILS_VER.tar.xz"
else
    echo "Binutils already extracted."
fi

if [ ! -d "$SOURCES/gcc-$GCC_VER" ]; then
    echo "Extracting GCC..."
    tar -xf "gcc-$GCC_VER.tar.xz"
else
    echo "GCC already extracted."
fi

if [ ! -d "$SOURCES/gmp-$GMP_VER" ]; then
    echo "Extracting GMP..."
    tar -xf "gmp-$GMP_VER.tar.xz"
else
    echo "GMP already extracted."
fi

if [ ! -d "$SOURCES/mpfr-$MPFR_VER" ]; then
    echo "Extracting MPFR..."
    tar -xf "mpfr-$MPFR_VER.tar.xz"
else
    echo "MPFR already extracted."
fi

if [ ! -d "$SOURCES/mpc-$MPC_VER" ]; then
    echo "Extracting MPC..."
    tar -xf "mpc-$MPC_VER.tar.gz"
else
    echo "MPC already extracted."
fi

if [ ! -d "$SOURCES/musl-$MUSL_VER" ]; then
    echo "Extracting musl..."
    tar -xf "musl-$MUSL_VER.tar.gz"
else
    echo "musl already extracted."
fi


# ============================================================
# Prepare GCC prerequisites
# ============================================================

echo
echo "========================================"
echo "Preparing GCC prerequisites"
echo "========================================"

cd "$SOURCES/gcc-$GCC_VER"

rm -rf gmp mpfr mpc

ln -s "$SOURCES/gmp-$GMP_VER" gmp
ln -s "$SOURCES/mpfr-$MPFR_VER" mpfr
ln -s "$SOURCES/mpc-$MPC_VER" mpc


# ============================================================
# Download target dependencies
# ============================================================

echo
echo "========================================"
echo "Downloading target dependencies"
echo "========================================"

"$SCRIPT_DIR/download_linux_headers.sh"

"$SCRIPT_DIR/download_libffi.sh"

"$SCRIPT_DIR/download_expat.sh"

"$SCRIPT_DIR/download_libxml2.sh"

"$SCRIPT_DIR/download_libxkbcommon.sh"

"$SCRIPT_DIR/download_wayland.sh"

"$SCRIPT_DIR/download_wayland-protocols.sh"

"$SCRIPT_DIR/download_libinput.sh

"$SCRIPT_DIR/download_libedev.sh
"$SCRIPT_DIR/download_udev.sh
"$SCRIPT_DIR/download_libcap.sh

# ============================================================
# Done
# ============================================================

echo
echo "========================================"
echo "Source download completed"
echo "========================================"

echo
echo "Sources directory:"
echo "$SOURCES"

echo
echo "Downloaded/extracted sources:"
ls -1 "$SOURCES"

echo
echo "========================================"
echo "DONE"
echo "========================================"