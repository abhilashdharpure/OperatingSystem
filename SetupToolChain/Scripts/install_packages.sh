sudo apt update
sudo apt install -y build-essential git wget curl bison flex gawk \
  libgmp-dev libmpfr-dev libmpc-dev libisl-dev texinfo pkg-config \
  python3 python3-pip autoconf automake libtool m4 \
  libx11-dev libxrandr-dev libxkbcommon-dev libxcb1-dev \
  libxcb-composite0-dev libxcb-xfixes0-dev libxcb-xinput-dev \
  libegl1-mesa-dev libgles2-mesa-dev libdrm-dev libgbm-dev \
  libinput-dev libwayland-dev wayland-protocols meson ninja-build \
  cmake xmlto llvm-13-dev libclang-13-dev
  
echo "Upgrading Meson build system..."
pip3 install --upgrade meson
