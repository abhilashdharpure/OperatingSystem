source ./Scripts/env.sh
cd $TOP/build
mkdir -p build-binutils && cd build-binutils
$TOP/src/binutils-$BINUTILS_VER/configure \
  --prefix=$PREFIX \
  --target=x86_64-linux-musl \
  --disable-multilib \
  --with-sysroot=$SYSROOT
make -j$(nproc)
make install
# Ensure toolchain bin is in PATH for the rest of the build
export PATH=$PREFIX/bin:$PATH
