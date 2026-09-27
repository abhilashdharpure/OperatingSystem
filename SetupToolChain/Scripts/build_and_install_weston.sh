#!/bin/bash
set -e
source ./env.sh

export WLD_PREFIX=$SYSROOT/usr
export PATH=$SYSROOT/usr/bin:$PATH
export PKG_CONFIG_PATH=$SYSROOT/usr/share/pkgconfig:$SYSROOT/usr/lib/x86_64-linux-gnu/pkgconfig

cd /home/abhilash/build/src/weston
rm -rf build
meson setup build \
  -Dprefix=$WLD_PREFIX \
  -Dbuildtype=release \
  -Dbackend-drm=true \
  -Dbackend-x11=false \
  -Dbackend-wayland=true \
  -Dbackend-vnc=false \
  -Dbackend-pipewire=false \
  -Dbackend-rdp=false \
  -Drenderer-vulkan=false \
  -Dtests=false \
  -Dimage-webp=false \
  -Dcolor-management-lcms=false \
  -Dsystemd=false \
  -Dshell-lua=false
ninja -C build
ninja -C build install
