source ./env.sh
export WLD_PREFIX=/home/abhilash/Project/OperatingSystem/sysroot/usr

cd ~/Project/OperatingSystem/src
wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.10.tar.xz
tar -xf linux-6.10.tar.xz
cd linux-6.10
make headers_install ARCH=x86_64 INSTALL_HDR_PATH=$SYSROOT/usr


cd ~/Project/OperatingSystem/src/libffi-3.4.4
make distclean
./configure --prefix=/usr --host=x86_64-linux-musl --disable-shared --enable-static --disable-trampolines
make
make install DESTDIR=$SYSROOT
