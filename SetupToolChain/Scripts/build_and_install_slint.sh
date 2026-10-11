#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"
source ~/.cargo/env

SRC="$SOURCES/slint-$SLINT_VER"
BUILD="$TOP/build-slint"

unset PKG_CONFIG_LIBDIR PKG_CONFIG_SYSROOT_DIR PKG_CONFIG_PATH

# 1) Native compiler (.slint -> C++ header). Runs on the host.
cargo install slint-compiler --version "$SLINT_VER" --root "$PREFIX/native" --locked

# 2) Cross-build the C++ runtime (static, software renderer only)
export CARGO_TARGET_X86_64_UNKNOWN_LINUX_MUSL_LINKER=$PREFIX/bin/$TARGET-gcc
export CC_x86_64_unknown_linux_musl=$PREFIX/bin/$TARGET-gcc
export CXX_x86_64_unknown_linux_musl=$PREFIX/bin/$TARGET-g++
export AR_x86_64_unknown_linux_musl=$PREFIX/bin/$TARGET-ar
export RUST_FONTCONFIG_DLOPEN=1

rm -rf "$BUILD"; mkdir -p "$BUILD"
cmake -G Ninja -S "$SRC" -B "$BUILD" \
  -DCMAKE_TOOLCHAIN_FILE="$SCRIPT_DIR/cmake/x86_64-linux-musl.cmake" \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DBUILD_SHARED_LIBS=OFF \
  -DRust_CARGO_TARGET=x86_64-unknown-linux-musl \
  -DSLINT_COMPILER="$PREFIX/native/bin/slint-compiler" \
  -DSLINT_BUILD_TESTING=OFF -DSLINT_BUILD_EXAMPLES=OFF \
  -DSLINT_FEATURE_ACCESSIBILITY=OFF \
  -DSLINT_FEATURE_SYSTEM_TRAY=OFF \
  -DSLINT_FEATURE_TESTING=OFF \
  -DSLINT_FEATURE_BACKEND_WINIT=OFF \
  -DSLINT_FEATURE_BACKEND_QT=OFF \
  -DSLINT_FEATURE_BACKEND_LINUXKMS=OFF \
  -DSLINT_FEATURE_RENDERER_FEMTOVG=OFF \
  -DSLINT_FEATURE_RENDERER_SKIA=OFF \
  -DSLINT_FEATURE_RENDERER_SOFTWARE=ON \
  -DSLINT_FEATURE_INTERPRETER=OFF \
  -DSLINT_FEATURE_FREESTANDING=OFF
cmake --build "$BUILD"
DESTDIR="$SYSROOT" cmake --install "$BUILD"
ls "$SYSROOT/usr/lib/libslint_cpp.a" "$SYSROOT/usr/include/slint/"