#!/bin/bash
set -e
source ./env.sh

export WLD_PREFIX=$SYSROOT/usr
export PATH=$SYSROOT/usr/bin:$PATH
export PKG_CONFIG_PATH=$SYSROOT/usr/lib/x86_64-linux-gnu/pkgconfig

cd $TOP/src
rm -rf libinput
git clone https://gitlab.freedesktop.org/libinput/libinput.git
cd libinput
meson setup build -Dprefix=$WLD_PREFIX -Dtests=false -Ddocumentation=false
ninja -C build
ninja -C build install
