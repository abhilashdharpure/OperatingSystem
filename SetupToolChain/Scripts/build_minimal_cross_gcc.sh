source ./Scripts/env.sh

echo "Install musl headers into the sysroot before building GCC stage‑1"
cd $TOP/src/musl-$MUSL_VER
./configure --prefix=/usr
make install-headers DESTDIR=$SYSROOT

cd $TOP/build
mkdir -p build-gcc-stage1 && cd build-gcc-stage1
$TOP/src/gcc-$GCC_VER/configure \
  --prefix=$PREFIX \
  --target=x86_64-linux-musl \
  --with-sysroot=$SYSROOT \
  --enable-languages=c \
  --disable-multilib \
  --disable-bootstrap \
  --disable-libsanitizer \
  --without-headers
make all-gcc -j$(nproc)
make install-gcc
# Verify minimal gcc exists
$PREFIX/bin/x86_64-linux-musl-gcc --version

cd $TOP/build/build-gcc-stage1
make all-target-libgcc -j$(nproc)
make install-target-libgcc
