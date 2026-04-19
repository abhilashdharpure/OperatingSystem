ENTRY(_start)

SECTIONS
{
  /* User text base */
  . = 0x40000000;

  /* Code */
  .text : ALIGN(0x1000)
  {
    *(.text .text.*)
    *(.plt .plt.*)
  }

  /* Read-only data */
  .rodata : ALIGN(0x1000)
  {
    *(.rodata .rodata.*)
    *(.gnu.linkonce.r.*)
  }

  /* Writable data (no fancy relro/GOT/TLS yet) */
  .data : ALIGN(0x1000)
  {
    *(.data .data.*)
    *(.gnu.linkonce.d.*)

    /* C++ init/fini arrays */
    . = ALIGN(8);
    __preinit_array_start = .;
    KEEP(*(.preinit_array*))
    __preinit_array_end = .;

    __init_array_start = .;
    KEEP(*(.init_array*))
    __init_array_end = .;

    __fini_array_start = .;
    KEEP(*(.fini_array*))
    __fini_array_end = .;
  }

  .bss : ALIGN(0x1000)
  {
    __bss_start = .;
    *(.bss .bss.*)
    *(COMMON)
    __bss_end = .;
  }

  _end = .;
}
