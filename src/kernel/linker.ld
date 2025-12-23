OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(multiboot_entry)

SECTIONS
{
    . = 0x0;
    .multiboot_header :
    {
        KEEP(*(.multiboot_header))
    }

    . = 0x100;
    .boot32 :
    {
        KEEP(*(.text.boot32))
    }

    . = 0x1000;
    .text :
    {
        *(.text*)
    }

    .rodata :
    {
        *(.rodata*)
    }

    .data :
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

    _kernel_start = 0x1000;
    _kernel_end   = .;
}
