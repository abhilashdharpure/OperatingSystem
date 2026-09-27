#!/bin/bash
set -e
source ./env.sh

export WLD_PREFIX=$SYSROOT/usr
export PATH=$SYSROOT/usr/bin:$PATH
export PKG_CONFIG_PATH=$SYSROOT/usr/lib/x86_64-linux-gnu/pkgconfig

cd $TOP/src
rm -rf wayland-protocols
git clone https://gitlab.freedesktop.org/wayland/wayland-protocols.git
cd wayland-protocols
git checkout main   # ensures latest version (1.49)
meson setup build --prefix=$SYSROOT/usr --buildtype=release -Dtests=false
ninja -C build
ninja -C build install
