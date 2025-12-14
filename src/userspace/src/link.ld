/* link.ld - Minimal linker script for 32-bit user-space init */

ENTRY(_start)           /* Entry point function */

/* Define memory layout */
SECTIONS
{
    . = 0x08048000;

    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss) *(COMMON) }
}
ENTRY(_start)
