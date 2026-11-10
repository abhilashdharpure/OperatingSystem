#!/usr/bin/env bash
set -e

ISO=${1:?usage: run.sh <iso> [live|disk|uefi]}
MODE=${2:-live}
ROOT_IMG="$(dirname "$ISO")/root.img"

[ -f "$ISO" ] || { echo "ISO not found: $ISO"; exit 1; }

case "$MODE" in
  live)   # RAM disk from the ISO only: what a USB stick looks like
    EXTRA=(-M pc -cdrom "$ISO" -boot d) ;;
  disk)   # IDE disk; choose the second GRUB entry to use it
    [ -f "$ROOT_IMG" ] || { echo "root.img not found: $ROOT_IMG"; exit 1; }
    EXTRA=(-M pc -cdrom "$ISO" -boot d -device piix3-ide
           -drive "file=$ROOT_IMG,format=raw,if=ide,index=0") ;;
  uefi)   # modern-laptop path: UEFI firmware (needs the 'ovmf' package)
    EXTRA=(-M q35 -bios /usr/share/ovmf/OVMF.fd -cdrom "$ISO" -boot d) ;;
  *) echo "mode must be live, disk or uefi"; exit 1 ;;
esac

echo "ISO: $ISO   mode: $MODE"
exec qemu-system-x86_64 -m 512M -serial mon:stdio -no-reboot "${EXTRA[@]}"