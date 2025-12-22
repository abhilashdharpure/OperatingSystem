#!/bin/bash

BINUTILS_VERSION="${BINUTILS_VERSION:-2.37}"
GCC_VERSION="${GCC_VERSION:-11.2.0}"

BINUTILS_URL="https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.xz"
GCC_URL="https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz"

TARGET="${TARGET:-x86_64-elf}"

set -e

TOOLCHAINS_DIR=
OPERATION='build'

while test $# -gt 0
do
    case "$1" in
        -c) OPERATION='clean' ;;
        *)  TOOLCHAINS_DIR=$(pwd)/$1 ;;
    esac
    shift
done

if [ -z "$TOOLCHAINS_DIR" ]; then
    echo "Usage: TARGET=x86_64-elf ./setup_toolchain.sh <dir>"
    exit 1
fi

mkdir -p "$TOOLCHAINS_DIR"
cd "$TOOLCHAINS_DIR"

TOOLCHAIN_PREFIX="$TOOLCHAINS_DIR/$TARGET"
mkdir -p "$TOOLCHAIN_PREFIX"

if [ "$OPERATION" = "build" ]; then

    # -------------------------
    # Binutils
    # -------------------------
    if [ ! -d "binutils-${BINUTILS_VERSION}" ]; then
        wget -c ${BINUTILS_URL}
        tar -xf binutils-${BINUTILS_VERSION}.tar.xz
    fi

    mkdir -p binutils-build-${TARGET}
    cd binutils-build-${TARGET}

    ../binutils-${BINUTILS_VERSION}/configure \
        --prefix="$TOOLCHAIN_PREFIX" \
        --target=$TARGET \
        --with-sysroot \
        --disable-nls \
        --disable-werror

    make -j$(nproc)
    make install
    cd ..

    # -------------------------
    # GCC (freestanding)
    # -------------------------
    if [ ! -d "gcc-${GCC_VERSION}" ]; then
        wget -c ${GCC_URL}
        tar -xf gcc-${GCC_VERSION}.tar.xz
    fi

    mkdir -p gcc-build-${TARGET}
    cd gcc-build-${TARGET}

    ../gcc-${GCC_VERSION}/configure \
        --prefix="$TOOLCHAIN_PREFIX" \
        --target=$TARGET \
        --disable-nls \
        --enable-languages=c,c++ \
        --without-headers \
        --disable-shared \
        --disable-threads \
        --disable-libssp \
        --disable-libstdcxx

    make -j$(nproc) all-gcc all-target-libgcc
    make install-gcc install-target-libgcc
    cd ..

elif [ "$OPERATION" = "clean" ]; then
    rm -rf binutils-build-* gcc-build-* $TARGET
fi
