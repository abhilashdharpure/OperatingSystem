#!/bin/bash

NATIVE_PREFIX="/home/abhilash/Project/OSDev/ToolchainWorkspace/Install/native"

unset PKG_CONFIG_SYSROOT_DIR
unset PKG_CONFIG_LIBDIR

export PKG_CONFIG_PATH="$NATIVE_PREFIX/lib/pkgconfig:$NATIVE_PREFIX/share/pkgconfig"

exec /usr/bin/pkg-config "$@"
