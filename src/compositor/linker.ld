/* linker.ld */
ENTRY(_start)

SECTIONS {
    . = 0x40000000;   /* high user base, above 1GiB huge page */

    .text : {
        *(.text*)
        *(.plt*)
    }

    .rodata : {
        *(.rodata*)
        *(.rodata.*)
    }

    .eh_frame_hdr : { *(.eh_frame_hdr) }
    .eh_frame     : { KEEP(*(.eh_frame)) }

    .preinit_array : {
        __preinit_array_start = .;
        KEEP(*(.preinit_array*))
        __preinit_array_end = .;
    }

    .init_array : {
        __init_array_start = .;
        KEEP(*(.init_array*))
        __init_array_end = .;
    }

    .fini_array : {
        __fini_array_start = .;
        KEEP(*(.fini_array*))
        __fini_array_end = .;
    }

    .data.rel.ro : { *(.data.rel.ro*) }

    .data : { *(.data*) }

    .tdata : { *(.tdata*) }
    .tbss  : { *(.tbss*) }

    .bss : {
        *(.bss*)
        *(COMMON)
    }
}