sudo apt install -y cmake ninja-build git curl pkg-config build-essential fonts-dejavu-core
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
source ~/.cargo/env
rustup update stable
rustup target add x86_64-unknown-linux-musl