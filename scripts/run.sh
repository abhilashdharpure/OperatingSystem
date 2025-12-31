#!/usr/bin/env bash
set -e

ISO=$1
if [[ -z "$ISO" ]]; then
    echo "Usage: $0 <iso_path>"
    exit 1
fi

BUILD_DIR=$(dirname "$ISO")
ROOT_IMG="$BUILD_DIR/root.img"

echo "ISO  : $ISO"
echo "ROOT : $ROOT_IMG"

if [ ! -f "$ISO" ]; then
    echo "ISO not found"
    exit 1
fi

if [ ! -f "$ROOT_IMG" ]; then
    echo "root.img not found"
    exit 1
fi

qemu-system-x86_64 \
  -M pc \
  -m 512M \
  -cdrom build/x86_64_debug/image.iso \
  -boot d \
  -device piix3-ide \
  -drive file=build/x86_64_debug/root.img,format=raw,if=ide,index=0 \
  -serial mon:stdio \
  -no-reboot \
  -monitor stdio






# qemu-system-x86_64 \
#     -M pc \
#     -m 512M \
#     -cdrom "$ISO" \
#     -hda "$ROOT_IMG" \
#     -serial mon:stdio \
#     -no-reboot

# qemu-system-x86_64 \
#     -M pc \
#     -m 512M \
#     -cdrom "$ISO" \
#     -hda "$ROOT_IMG" \
#     -serial mon:stdio \
#     -no-reboot


# #!/bin/bash

# if [ "$#" -ne 2 ]; then
#     echo "Usage: $0 <image_type> <image_file>"
#     exit 1
# fi

# IMAGE_TYPE="$1"
# IMAGE_FILE="$2"

# # sanity check
# if [ ! -f "$IMAGE_FILE" ]; then
#     echo "Error: ISO file not found: $IMAGE_FILE"
#     exit 1
# fi

# # Boot ISO as a virtual IDE hard disk
# qemu-system-x86_64 \
#     -m 512 \
#     -drive file="$IMAGE_FILE",format=raw,if=ide \
#     -serial mon:stdio \
#     -no-reboot