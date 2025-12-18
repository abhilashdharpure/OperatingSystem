ENTRY(start)
OUTPUT_FORMAT("elf32-i386")
phys = 0x00100000;

SECTIONS
{
    . = phys;
    _kernel_start = .;
    
    .text               : { __text_start = .;       *(.text)    }
    .data               : { __data_start = .;       *(.data)    }
    .rodata             : { __rodata_start = .;     *(.rodata)  }

    .bss :
    {
        __bss_start = .;
        *(.bss)
        *(COMMON)
        __bss_end = .;
    }

    /* Kernel stack (16 KiB) */
    . = ALIGN(16);
    _kernel_stack_bottom = .;
    . += 16384;
    _kernel_stack_top = .;

    
    __end = .;
    _kernel_end = .;
}

