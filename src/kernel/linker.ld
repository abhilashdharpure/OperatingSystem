OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(multiboot_entry)

/* Kernel lives at low virtual addresses, identity mapped */
KERNEL_PHYS_BASE = 0x00100000;   /* 1 MiB */

SECTIONS
{
    /* Multiboot header at 0 */
    .multiboot_header 0x00000000 : AT(0x00000000)
    {
        KEEP(*(.multiboot_header))
    }

    /* Early boot code at 1 MiB */
    .boot KERNEL_PHYS_BASE : AT(KERNEL_PHYS_BASE)
    {
        KEEP(*(.boot32))
        KEEP(*(.boot64_stub))
    }

    /* Early boot data (page tables, etc.) */
    .data.boot (KERNEL_PHYS_BASE + 0x1000) : AT(KERNEL_PHYS_BASE + 0x1000)
    {
        KEEP(*(.data.boot))
    }

    /* Mark kernel start */
    _kernel_start = .;

    /* Kernel .text */
    .text ALIGN(0x1000) : AT(ADDR(.text))
    {
        *(.text*)
    }

    /* Read‑only data */
    .rodata ALIGN(0x1000) : AT(ADDR(.rodata))
    {
        *(.rodata*)
    }

    /* Writable data */
    .data ALIGN(0x1000) : AT(ADDR(.data))
    {
        *(.data*)
    }

    /* BSS */
    .bss ALIGN(0x1000) (NOLOAD) :
    {
        *(.bss*)
        *(COMMON)
    }

    /* Kernel stack */
    . = ALIGN(16);
    _kernel_stack_bottom = .;
    . += 0x4000;        /* 16 KB stack */
    _kernel_stack_top = .;

    /* Kernel end */
    _kernel_end = .;
}
