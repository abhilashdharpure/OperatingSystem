#!/bin/bash
set -e

IMG=build/i686_debug/image.img
SIZE=250M
MNT=/mnt/osimg

echo "[1] Creating empty image $IMG..."
mkdir -p build/i686_debug
dd if=/dev/zero of="$IMG" bs=1M count=250

echo "[2] Partitioning (FAT32 LBA)..."
parted "$IMG" --script mklabel msdos
parted "$IMG" --script mkpart primary fat32 1MiB 100%
parted "$IMG" --script set 1 boot on

echo "[3] Creating loop device..."
LOOP=$(sudo losetup --find --show "$IMG")
echo "Loop device: $LOOP"

echo "[4] Creating partition mappings..."
sudo kpartx -av "$LOOP"

PART=/dev/mapper/$(basename "$LOOP")p1
echo "Partition: $PART"

echo "[5] Formatting as FAT32..."
sudo mkfs.vfat -F 32 "$PART"

echo "[6] Mounting..."
sudo mkdir -p "$MNT"
sudo mount "$PART" "$MNT"

echo "[7] Creating directory structure..."
sudo mkdir -p "$MNT/bin"
sudo mkdir -p "$MNT/boot"

echo "[8] Copying init..."
sudo cp build/i686_debug/user/luma-compositor "$MNT/bin/init"
sudo chmod +x "$MNT/bin/init"

echo "[9] Copying STAGE2.BIN..."
sudo cp build/i686_debug/stage2/stage2.bin "$MNT/STAGE2.BIN"
sudo chmod 644 "$MNT/STAGE2.BIN"

echo "[10] Sync + unmount..."
sync
sudo umount "$MNT"

echo "[11] Remove mapper..."
sudo kpartx -d "$LOOP"
sudo losetup -d "$LOOP"

echo "DONE. FAT32 image ready with /bin/init"
