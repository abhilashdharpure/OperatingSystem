#!/bin/bash

# ============================================================================
# OS Source Tree
# ============================================================================
export OSROOT=$HOME/Project/OSDev/OperatingSystem

# ============================================================================
# Toolchain Workspace (outside repository but inside OSDev)
# ============================================================================
export WORKSPACE=$HOME/Project/OSDev/ToolchainWorkspace

# Downloaded source packages
export SOURCES=$WORKSPACE/Sources

# Temporary build directories
export TOP=$WORKSPACE/Build

# Cross compiler installation
export PREFIX=$WORKSPACE/Install

# Target root filesystem
export SYSROOT=$WORKSPACE/Sysroot

# Build logs
export LOGS=$WORKSPACE/Logs

# ============================================================================
# Target
# ============================================================================
export TARGET=x86_64-linux-musl

# ============================================================================
# Package Versions
# ============================================================================
export BINUTILS_VER=2.42
export GCC_VER=13.3.0
export GMP_VER=6.3.0
export MPFR_VER=4.2.1
export MPC_VER=1.3.1
export MUSL_VER=1.2.6

export LINUX_VER=6.10
export LIBFFI_VER=3.4.4
export EXPAT_VER=2.6.3
export LIBXML2_VER=2.14.6
export LIBXKBCOMMON_VER=1.13.1
export WAYLAND_VER=1.24.91
export WAYLAND_PROTOCOLS_VER=1.44
export LIBINPUT_VER=1.29.1
export LIBEVDEV_VER=1.13.7
export SYSTEMD_VER=257.4
export LIBCAP_VER=2.76
export UTIL_LINUX_VER=2.40.4
export LIBXKBCOMMON_VER=1.8.1

export TARGET=x86_64-linux-musl

export MESON_CROSS_FILE="$OSROOT/SetupToolChain/Scripts/meson/x86_64-linux-musl.ini"

# ============================================================================
# Toolchain Path
# ============================================================================
export PATH=$PREFIX/bin:$PATH
export PATH=$PREFIX/native/bin:$PREFIX/bin:$PATH

# ===========================================