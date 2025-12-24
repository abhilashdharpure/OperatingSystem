#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <image_type> <image_file>"
    exit 1
fi

IMAGE_TYPE="$1"
IMAGE_FILE="$2"

# sanity check
if [ ! -f "$IMAGE_FILE" ]; then
    echo "Error: ISO file not found: $IMAGE_FILE"
    exit 1
fi

# Boot ISO as a virtual IDE hard disk
qemu-system-x86_64 \
    -m 512 \
    -drive file="$IMAGE_FILE",format=raw,if=ide \
    -serial mon:stdio \
    -no-reboot