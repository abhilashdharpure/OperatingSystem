#!/bin/bash
set -e
source ./env.sh

export WLD_PREFIX=$SYSROOT/usr
export PATH=$SYSROOT/usr/bin:$PATH
export PKG_CONFIG_PATH=$SYSROOT/usr/share/pkgconfig:$SYSROOT/usr/lib/x86_64-linux-gnu/pkgconfig

# Build seatd/libseat only if not already cloned
cd $SOURCES
if [ ! -d "seatd" ]; then
  git clone https://git.sr.ht/~kennylevinsen/seatd
fi
cd seatd
rm -rf build
meson setup build --prefix=$SYSROOT/usr --buildtype=release
ninja -C build
ninja -C build install


cd $SOURCES
git clone https://github.com/mm2/Little-CMS.git lcms2
cd lcms2
rm -rf build
cmake -B build -DCMAKE_INSTALL_PREFIX=$SYSROOT/usr
cmake --build build
cmake --install build