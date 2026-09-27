
#!/bin/bash
set -e
source ./env.sh

export WLD_PREFIX=$SYSROOT/usr
export PATH=$SYSROOT/usr/bin:$PATH
export PKG_CONFIG_PATH=$SYSROOT/usr/share/pkgconfig:$SYSROOT/usr/lib/x86_64-linux-gnu/pkgconfig

cd $TOP/src
wget https://www.lua.org/ftp/lua-5.4.7.tar.gz
tar xvf lua-5.4.7.tar.gz
cd lua-5.4.7
make linux
make INSTALL_TOP=$SYSROOT/usr install
