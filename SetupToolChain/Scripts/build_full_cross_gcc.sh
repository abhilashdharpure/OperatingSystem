source ./env.sh

cd $TOP/build
mkdir -p build-gcc-stage2 && cd build-gcc-stage2
$TOP/src/gcc-$GCC_VER/configure \
  --prefix=$PREFIX \
  --target=x86_64-linux-musl \
  --with-sysroot=$SYSROOT \
  --enable-languages=c,c++ \
  --disable-multilib \
  --disable-bootstrap \
  --disable-shared \
  --enable-threads=posix

make -j$(nproc) all-gcc all-target-libgcc
make install-gcc install-target-libgcc
# Now build and install libstdc++
make -j$(nproc) all-target-libstdc++-v3
make install-target-libstdc++-v3
# Verify
$PREFIX/bin/x86_64-linux-musl-g++ --version
