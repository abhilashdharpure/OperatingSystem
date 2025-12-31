/* link.ld - 64-bit user-space ELF */

ENTRY(_start)

SECTIONS
{
    . = 0x0000000000400000;

    .text : { *(.text*) }
    .rodata : { *(.rodata*) }
    .data : { *(.data*) }
    .bss : { *(.bss*) *(COMMON) }
}
