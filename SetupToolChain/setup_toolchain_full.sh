#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "========================================"
echo "Custom OS Toolchain Setup"
echo "========================================"
echo


# ============================================================
# Workspace
# ============================================================

echo "Creating ToolchainWorkspace..."

mkdir -p ~/Project/OSDev/ToolchainWorkspace/{Sources,Build,Install,Sysroot,Logs}

echo


# ============================================================
# Host dependencies
# ============================================================

echo "Installing required host packages..."

"$SCRIPT_DIR/Scripts/install_packages.sh"

echo


# ============================================================
# Download ALL sources
# ============================================================

echo "Downloading all source files..."

"$SCRIPT_DIR/Scripts/download_source_for_toolchain.sh"

echo


# ============================================================
# Bootstrap toolchain
# ============================================================

echo "Building and installing binutils..."

"$SCRIPT_DIR/Scripts/build_and_install_binutils.sh"

echo


echo "Building minimal cross GCC..."

"$SCRIPT_DIR/Scripts/build_minimal_cross_gcc.sh"

echo


# ============================================================
# Linux kernel UAPI headers
# ============================================================

echo "Installing Linux kernel UAPI headers..."

"$SCRIPT_DIR/Scripts/install_linux_headers.sh"

echo


# ============================================================
# musl
# ============================================================

echo "Building and installing musl..."

"$SCRIPT_DIR/Scripts/build_and_install_musl.sh"

echo


# ============================================================
# Full GCC
# ============================================================

echo "Building full cross GCC..."

"$SCRIPT_DIR/Scripts/build_full_cross_gcc.sh"

echo


# ============================================================
# Target libraries
# ============================================================

echo "Building and installing libffi..."

"$SCRIPT_DIR/Scripts/build_and_install_libffi.sh"

echo


echo "Building and installing Expat..."

"$SCRIPT_DIR/Scripts/build_and_install_expat.sh"

echo


echo "Building and installing libxml2..."

"$SCRIPT_DIR/Scripts/build_and_install_libxml2.sh"

echo


echo "Building and installing libxkbcommon..."

"$SCRIPT_DIR/Scripts/build_and_install_libxkbcommon.sh"

echo


# ============================================================
# Wayland
# ============================================================

echo "Building and installing Wayland..."

"$SCRIPT_DIR/Scripts/build_and_install_wayland.sh"

echo


echo "Building and installing Wayland protocols..."

"$SCRIPT_DIR/Scripts/build_and_install_wayland-protocols.sh"

echo

"$SCRIPT_DIR/Scripts/build_and_install_libxkbcommon_wayland.sh

# ============================================================
# Input / Graphics
# ============================================================


echo "Building and installing libevdev..."

"$SCRIPT_DIR/Scripts/build_and_install_libevdev.sh"

echo


echo "Building and installing libcap..."

"$SCRIPT_DIR/Scripts/build_and_install_libcap.sh"

echo

echo "Configuring libudev..."

"$SCRIPT_DIR/Scripts/configure_libudev.sh"

echo

echo "Creating musl printf header..."

"$SCRIPT_DIR/Scripts/create_musl_printf_h.sh"

echo

echo "Creating create_musl_printf_parser..."

"$SCRIPT_DIR/Scripts/create_musl_printf_parser.sh"

echo


echo "Creating configure_systemd_musl_printf..."

"$SCRIPT_DIR/Scripts/configure_systemd_musl_printf.sh"

echo


echo "Creating create_systemd_musl_compat_h..."

"$SCRIPT_DIR/Scripts/create_systemd_musl_compat_h.sh"

echo


## Setup Done till here...


echo "Building and installing udev..."

"$SCRIPT_DIR/Scripts/build_and_install_udev.sh"

echo

echo "Building and installing libinput..."

"$SCRIPT_DIR/Scripts/build_and_install_libinput.sh"

echo


echo "Building and installing Mesa..."

"$SCRIPT_DIR/Scripts/build_and_install_mesa.sh"

echo


echo "Building and installing other dependencies..."

"$SCRIPT_DIR/Scripts/build_and_install_other.sh"

echo


echo "Building and installing Lua..."

"$SCRIPT_DIR/Scripts/build_and_install_lua.sh"

echo


# ============================================================
# Weston
# ============================================================

echo "Building and installing Weston..."

"$SCRIPT_DIR/Scripts/build_and_install_weston.sh"

echo


# ============================================================
# Finished
# ============================================================

source "$SCRIPT_DIR/Scripts/env.sh"

echo "========================================"
echo "Custom OS Toolchain Setup Completed"
echo "========================================"

echo
echo "Target : $TARGET"
echo "Prefix : $PREFIX"
echo "Sysroot: $SYSROOT"

echo
echo "Toolchain:"
"$PREFIX/bin/$TARGET-gcc" --version | head -1
"$PREFIX/bin/$TARGET-g++" --version | head -1

echo
echo "========================================"
echo "DONE"
echo "========================================"