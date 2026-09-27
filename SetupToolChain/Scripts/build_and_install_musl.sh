source ./Scripts/env.sh


cd $TOP/src/musl-$MUSL_VER
./configure --prefix=/usr
make -j$(nproc)
make DESTDIR=$SYSROOT install
