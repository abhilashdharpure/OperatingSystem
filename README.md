# Project Layouyt
~/Project/OSDev/
├── OperatingSystem/
│   ├── Kernel/
│   ├── Userland/
│   ├── SetupToolChain/
│   └── ...
│
└── ToolchainWorkspace/
    ├── Sources/
    ├── Build/
    ├── Install/
    ├── Sysroot/
    └── Logs/

# Run In Host Machine
RAM:    ./scripts/run.sh build/x86_64_debug/image.iso live
Normal  scons run

# Create Boot image
Press F2 and disable secure boot in Laptop
lsblk -o NAME,SIZE,MODEL,TRAN,RM,MOUNTPOINT               # the stick: TRAN=usb, RM=1, matching size
sudo umount /dev/sdX* 2>/dev/null
sudo dd if=build/x86_64_debug/image.iso of=/dev/sdX bs=4M conv=fsync status=progress
sync

# Icons 
  https://www.svgrepo.com/

# Dependency Flow
                    ┌── binutils
                    │
                    └── stage-1 GCC
                           │
                           ▼
                    Linux UAPI headers
                           │
                           ▼
                         musl
                           │
                           ▼
                      Full GCC
                       C + C++
                           │
             ┌─────────────┼─────────────┐
             ▼             ▼             ▼
           libffi        Expat        libxml2
             │             │             │
             └─────────────┴──────┬──────┘
                                  ▼
                           libxkbcommon
                                  │
                                  ▼
                              Wayland
                                  │
                                  ▼
                        Wayland protocols
                                  │
                    ┌─────────────┴─────────────┐
                    ▼                           ▼
                 libinput                     Mesa
                    │                           │
                    └─────────────┬─────────────┘
                                  ▼
                           other dependencies
                                  │
                                  ▼
                                Lua
                                  │
                                  ▼
                                Weston
    
## Building

First, install the following dependencies:

```sh
# Ubuntu, Debian:
sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo wget \
                   nasm mtools python3 python3-pip python3-parted scons dosfstools libguestfs-tools qemu-system-x86

# Fedora:
sudo dnf install gcc gcc-c++ make bison flex gmp-devel libmpc-devel mpfr-devel texinfo wget \
                   nasm mtools python3 python3-pip python3-pyparted python3-scons dosfstools guestfs-tools qemu-system-x86

# Arch & Arch-based:
paru -S gcc make bison flex libgmp-static libmpc mpfr texinfo nasm mtools qemu-system-x86 python3 scons
```
NOTE: to install all the required packages on Arch, you need an [AUR helper](https://wiki.archlinux.org/title/AUR_helpers).

Then you must run `python3 -m pip install -r requirements.txt`

Next, modify the configuration in `build_scripts/config.py`. The most important is the `toolchain='../.toolchains'` option which sets where the toolchain will be downloaded and built. The default option is in the directory above where the repo is cloned, in a .toolchains directory, but you will get an error if this directory doesn't exist.

After that, run `scons toolchain`, this should download and build the required tools (binutils and GCC). If you encounter errors during this step, you might have to modify `scripts/setup_toolchain.sh` and try a different version of **binutils** and **gcc**. Using the same version as the one bundled with your distribution is your best bet.

Finally, you should be able to run `scons`. Use `scons run` to test your OS using qemu.

## Links

* [Discord channel](https://discord.gg/RgHc5XrCEw)
* [Patreon](https://www.patreon.com/nanobyte)
