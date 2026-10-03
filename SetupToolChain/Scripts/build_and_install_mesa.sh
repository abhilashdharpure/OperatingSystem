#!/bin/bash
set -e
source ./env.sh

export WLD_PREFIX=$SYSROOT/usr
export PATH=$SYSROOT/usr/bin:$PATH
export PKG_CONFIG_PATH=$SYSROOT/usr/lib/x86_64-linux-gnu/pkgconfig

cd $SOURCES
rm -rf mesa
git clone https://gitlab.freedesktop.org/mesa/mesa.git
cd mesa
meson setup build \
  -Dprefix=$WLD_PREFIX \
  -Dbuildtype=release \
  -Dglx=disabled \
  -Degl=enabled \
  -Dgbm=enabled \
  -Dopengl=true \
  -Dgles1=disabled \
  -Dgles2=enabled \
  -Dgallium-drivers=llvmpipe \
  -Dvulkan-drivers=[] \
  -Dplatforms=wayland
ninja -C build
ninja -C build install
