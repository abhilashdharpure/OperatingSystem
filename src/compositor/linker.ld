ENTRY(_start)

SECTIONS {
  /* Start user code at 0x40000000, but skip one page for ELF headers */
  . = 0x40001000;

  .text : {
    *(.text .text.*)
    *(.plt .plt.*)
    *(.init)
    *(.fini)
  }

  .rodata : ALIGN(0x1000) { *(.rodata .rodata.*) }
  .data   : ALIGN(0x1000) { *(.data .data.*) }
  .bss    : ALIGN(0x1000) { *(.bss .bss.*) *(COMMON) }
  _end = .;
}
