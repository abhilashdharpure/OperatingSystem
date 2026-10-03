#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

SRC_DIR="$SOURCES/systemd-$SYSTEMD_VER"
BUILD_DIR="$TOP/build-systemd-libudev"

export PKG_CONFIG_SYSROOT_DIR="$SYSROOT"
export PKG_CONFIG_LIBDIR="$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/share/pkgconfig:$SYSROOT/usr/lib64/pkgconfig"
unset PKG_CONFIG_PATH

if [ ! -d "$SRC_DIR" ]; then
    echo "ERROR: systemd source directory not found:"
    echo "  $SRC_DIR"
    echo
    echo "Run:"
    echo "  $SCRIPT_DIR/download_udev.sh"
    exit 1
fi

rm -rf "$BUILD_DIR"

echo "========================================"
echo "Configuring systemd $SYSTEMD_VER"
echo "for target libudev"
echo "========================================"
echo
echo "Source:"
echo "  $SRC_DIR"
echo
echo "Build:"
echo "  $BUILD_DIR"
echo
echo "Target:"
echo "  $TARGET"
echo
echo "Sysroot:"
echo "  $SYSROOT"
echo

meson setup \
    "$BUILD_DIR" \
    "$SRC_DIR" \
    --cross-file "$MESON_CROSS_FILE" \
    --prefix=/usr \
    --buildtype=release \
    \
    -Dmode=release \
    -Dtests=false \
    -Dman=disabled \
    -Dhtml=disabled \
    -Dtranslations=false \
    \
    -Dinitrd=false \
    -Dhibernate=false \
    -Dresolve=false \
    -Dnetworkd=false \
    -Dlogind=false \
    -Dhostnamed=false \
    -Dlocaled=false \
    -Dmachined=false \
    -Dportabled=false \
    -Dtimesyncd=false \
    -Dtimedated=false \
    -Duserdb=false \
    -Dhomed=disabled \
    -Doomd=false \
    -Dcoredump=false \
    -Dpstore=false \
    -Dremote=disabled \
    -Dimportd=disabled \
    -Dsysusers=false \
    -Dgshadow=false \
    -Dtmpfiles=false \
    -Drandomseed=false \
    -Dbacklight=false \
    -Dvconsole=false \
    -Dfirstboot=false \
    -Dbinfmt=false \
    -Drepart=disabled \
    -Dsysupdate=disabled \
    -Dsysupdated=disabled \
    -Dmountfsd=false \
    -Dnsresourced=false \
    \
    -Dnss-systemd=false \
    -Dnss-myhostname=false \
    -Dnss-mymachines=disabled \
    -Dnss-resolve=disabled \
    \
    -Defi=false \
    -Dtpm=false \
    \
    -Dseccomp=disabled \
    -Dselinux=disabled \
    -Dapparmor=disabled \
    -Dsmack=false \
    -Dima=false \
    -Dipe=false \
    -Dacl=disabled \
    -Daudit=disabled \
    -Dkmod=disabled \
    -Dpam=disabled \
    -Dlibcryptsetup=disabled \
    -Dlibcryptsetup-plugins=disabled \
    -Dlibcurl=disabled \
    -Dqrencode=disabled \
    -Dgcrypt=disabled \
    -Dgnutls=disabled \
    -Dopenssl=disabled \
    -Dp11kit=disabled \
    -Dlibfido2=disabled \
    -Dtpm2=disabled \
    -Delfutils=disabled \
    -Dxkbcommon=disabled \
    -Dglib=disabled \
    \
    -Dhwdb=true \
    \
    -Dstatic-libsystemd=false \
    -Dstatic-libudev=false \
    \
    -Dlink-udev-shared=false

echo
echo "========================================"
echo "Configuration complete"
echo "========================================"
echo
echo "Build directory:"
echo "  $BUILD_DIR"
echo
echo "Now inspect the configuration with:"
echo
echo "  meson configure $BUILD_DIR"
echo
echo "Do NOT build yet."
