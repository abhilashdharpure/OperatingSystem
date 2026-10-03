sudo apt update
sudo apt install -y \
    build-essential \
    git \
    wget \
    curl \
    bison \
    flex \
    gawk \
    texinfo \
    pkg-config \
    python3 \
    python3-pip \
    python3-setuptools \
    python3-mako \
    python3-jinja2 \
    autoconf \
    automake \
    libtool \
    m4 \
    ninja-build \
    meson \
    cmake \
    xmlto \
    gettext \
    gperf \
    libgmp-dev \
    libmpfr-dev \
    libmpc-dev \
    libisl-dev \
    libffi-dev \
    libexpat1-dev \
    libxml2-dev \
    llvm-dev \
    libclang-dev \
    clang \
    lld
    
echo "Upgrading Meson build system..."
pip3 install --upgrade meson
