OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(multiboot_entry)

/* Kernel will live at low virtual addresses, identity mapped */
KERNEL_PHYS_BASE = 0x00100000;   /* 1 MiB, for example */

SECTIONS
{
    /* Multiboot header at 0 */
    .multiboot_header 0x00000000 : AT(0x00000000)
    {
        KEEP(*(.multiboot_header))
    }

    /* Early 32-bit and 64-bit boot code at 0x00100000 */
    .boot 0x00100000 : AT(0x00100000)
    {
        KEEP(*(.boot32))
        KEEP(*(.boot64_stub))
    }

    /* Page tables at 0x00101000 */
    .data.boot 0x00101000 : AT(0x00101000)
    {
        KEEP(*(.data.boot))
    }

    /* Mark kernel start */
    _kernel_start = .;

    /* Rest of the kernel just after .boot, all low, identity-mapped */
    .text : AT(ADDR(.text))
    {
        *(.text*)
    }

    .rodata : AT(ADDR(.rodata))
    {
        *(.rodata*)
    }

    .data : AT(ADDR(.data))
    {
        *(.data*)
    }

    .bss (NOLOAD) :
    {
        *(.bss*)
        *(COMMON)
    }

    . = ALIGN(16);
    _kernel_stack_bottom = .;
    . += 0x4000;
    _kernel_stack_top = .;

    _kernel_end = .;
}
