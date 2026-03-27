
scons -c
clear
echo "Copying Userspace, Compositor to rootfs"
./copy_userspace.sh
echo "Starting Kernel"
scons run
