source ./env.sh
export WLD_PREFIX=/home/abhilash/Project/OperatingSystem/sysroot/usr

cd ~/Project/OperatingSystem/src
rm -rf wayland
git clone https://gitlab.freedesktop.org/wayland/wayland.git
cd wayland
meson setup build --prefix=$WLD_PREFIX --buildtype=release -Ddocumentation=false
ninja -C build
ninja -C build install
