
echo "Installing required packages..."
./Scripts/install_packages.sh


echo "Downloading source files..."
./Scripts/download_source_for_toolchain.sh


echo "Building and installing binutils..."
./Scripts/build_and_install_binutils.


echo "Building minimal cross GCC..."
./Scripts/build_minimal_cross_gcc.sh


echo "Building and installing musl..."
./Scripts/build_and_install_musl.sh


echo "Rebuild GCC with C and C++ (full toolchain)..."
./Scripts/build_full_cross_gcc.sh


echo "Building and installing Wayland..."
./Scripts/build_and_install_wayland.sh


echo "Building and installing Wayland protocols..."
./Scripts/build_and_install_wayland-protocols.sh


echo "Building and installing libinput..."
./Scripts/build_and_install_libinput.sh


echo "Building and installing Mesa..."
./Scripts/build_and_install_mesa.sh

./Scripts/build_and_install_other.sh

./Scriptsc/build_and_install_lua.sh

echo "Building and installing Weston..."
./Scripts/build_and_install_weston.sh


#Place this file at $SYSROOT/etc/xdg/weston/weston.ini:


echo "Building and installing libxkbcommon..."
./Scripts/build_and_install_libxkbcommon.sh