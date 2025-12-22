OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(multiboot_entry)

SECTIONS
{
    /* -------------------------------------------------- */
    /* Multiboot header must be very early                */
    /* -------------------------------------------------- */
    .multiboot_header 0x0 : AT(0x0)
    {
        KEEP(*(.multiboot_header))
    }

    /* -------------------------------------------------- */
    /* 32-bit boot code (runs before paging)              */
    /* -------------------------------------------------- */
    . = 0x100;
    .boot32 : AT(0x100)
    {
        KEEP(*(.text.boot32))
    }

    /* -------------------------------------------------- */
    /* Kernel proper (identity mapped)                    */
    /* -------------------------------------------------- */
    . = 0x1000;
    .text : AT(0x1000)
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

    .bss (NOLOAD) : AT(ADDR(.bss))
    {
        *(.bss*)
        *(COMMON)
    }

    /* -------------------------------------------------- */
    /* Kernel stack (NOLOAD, but identity mapped)         */
    /* -------------------------------------------------- */
    .stack (NOLOAD) : AT(ADDR(.bss) + SIZEOF(.bss))
    {
        _kernel_stack_bottom = .;
        . += 0x4000;
        _kernel_stack_top = .;
    }

    /* -------------------------------------------------- */
    /* Useful symbols                                     */
    /* -------------------------------------------------- */
    _kernel_start = 0x1000;
    _kernel_end   = .;
}
