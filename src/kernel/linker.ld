OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(multiboot_entry)

/* Physical load addresses */
BOOT_LMA_BASE   = 0x0000000000100000;  /* 1 MiB */
KERNEL_LMA_BASE = 0x0000000000200000;  /* 2 MiB */

/* Higher-half virtual base for kernel proper */
KERNEL_VMA_BASE = 0xffffffff80000000;

SECTIONS
{
    /* Multiboot header at 0, identity-mapped */
    .multiboot_header 0x00000000 : AT(0x00000000)
    {
        KEEP(*(.multiboot_header))
    }

    /* 32-bit entry at 1 MiB, identity-mapped */
    .boot32 BOOT_LMA_BASE : AT(BOOT_LMA_BASE)
    {
        KEEP(*(.boot32))
    }

    /* Early boot data (page tables, GDT, etc.), identity-mapped */
    .data.boot (BOOT_LMA_BASE + 0x1000) : AT(BOOT_LMA_BASE + 0x1000)
    {
        KEEP(*(.data.boot))
    }

    /* 64-bit low stub placed after data.boot, no overlap */
    .boot64_stub : AT(LOADADDR(.data.boot) + SIZEOF(.data.boot))
    {
        KEEP(*(.boot64_stub))

        /* low, identity-mapped boot stack */
        . = ALIGN(16);
        _boot_stack_bottom = .;
        . += 0x2000;          /* 8 KiB boot stack */
        _boot_stack_top = .;
    }

    .boot64_stub_data : AT(LOADADDR(.boot64_stub) + SIZEOF(.boot64_stub))
    {
        *(.boot64_stub_data)
    }

    /* end of all low stub code+data */
    _boot_stub_end = .;

    /* Drop all unwind / exception metadata */
    /DISCARD/ :
    {
        *(.eh_frame)
        *(.eh_frame.*)
        *(.gcc_except_table)
        *(.gcc_except_table.*)
    }

    /* Higher-half kernel proper */
    . = KERNEL_VMA_BASE;
    _kernel_start = .;

    .text : AT(KERNEL_LMA_BASE)
    {
        *(.text*)
    }

    .rodata ALIGN(0x1000) : AT(ADDR(.rodata) - KERNEL_VMA_BASE + KERNEL_LMA_BASE)
    {
        *(.rodata*)
    }

    .data ALIGN(0x1000) : AT(ADDR(.data) - KERNEL_VMA_BASE + KERNEL_LMA_BASE)
    {
        *(.data*)
    }

    .bss ALIGN(0x1000) (NOLOAD) :
    {
        *(.bss*)
        *(COMMON)
    }

    _kernel_phys_start = LOADADDR(.text);
    _kernel_phys_end   = LOADADDR(.bss) + SIZEOF(.bss);


    . = ALIGN(16);
    _kernel_stack_bottom = .;
    . += 0x4000;
    _kernel_stack_top = .;

    _kernel_end = .;
}
