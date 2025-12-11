/* link.ld - Minimal linker script for 32-bit user-space init */

ENTRY(_start)           /* Entry point function */

/* Define memory layout */
SECTIONS
{
    /* Place text/code at 0x08048000 */
    . = 0x8b9080c0;
    
    .text : {
        *(.text)       /* All code sections from object files */
    }

    .rodata : {
        *(.rodata)     /* Read-only data */
    }

    .data : {
        *(.data)       /* Initialized data */
    }

    .bss : {
        *(.bss)        /* Zero-initialized data */
        *(COMMON)
    }
}
