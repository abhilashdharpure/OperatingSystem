source ./env.sh
export PREFIX=$SYSROOT/usr

rm -rf libxkbcommon-xkbcommon-1.13.1

# ##How to cross‑compile Wayland (server library)
# Download wayland and ibxkbcommon/:
# https://gitlab.freedesktop.org/wayland/wayland/-/releases
# https://github.com/xkbcommon/libxkbcommon/releases


# mv ~/Downloads/libxkbcommon-xkbcommon-1.13.1.tar.gz /home/abhilash/Project/OperatingSystem/SetupToolChain/Scripts
cp ~/Project/OperatingSystem/src/libs/libxkbcommon-xkbcommon-1.13.1.tar.gz /home/abhilash/Project/OperatingSystem/SetupToolChain/Scripts
tar -xvf libxkbcommon-xkbcommon-1.13.1.tar.gz 

# git clone git@gitlab.freedesktop.org:xkbcommon/libxkbcommon.git

cd libxkbcommon-xkbcommon-1.13.1

# meson setup build --prefix=$PREFIX --buildtype=release -Denable-x11=false
meson setup build --prefix=$PREFIX --buildtype=release -Denable-x11=false -Dbash-completion=false

ninja -C build
ninja -C build install
