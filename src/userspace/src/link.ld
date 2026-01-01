ENTRY(_start)

SECTIONS
{
    . = 0x40000000;

    .text : {
        *(.text.crt0)   /* ensure crt0 comes first */
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        *(.bss*)
        *(COMMON)
    }
}
