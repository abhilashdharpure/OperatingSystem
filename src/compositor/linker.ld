ENTRY(_start)

SECTIONS
{
  /* Move user text base above identity-mapped low 1GiB */
  . = 0x40200000;   /* or 0x41000000, anything safely > 0x40000000 + 2MiB */

  .text : ALIGN(0x1000)
  {
    *(.text .text.*)
    *(.plt .plt.*)
  }

  .rodata : ALIGN(0x1000)
  {
    *(.rodata .rodata.*)
    *(.gnu.linkonce.r.*)
  }

  .data : ALIGN(0x1000)
  {
    *(.data .data.*)
    *(.gnu.linkonce.d.*)

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
