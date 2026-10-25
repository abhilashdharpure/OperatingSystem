ENTRY(_start)

SECTIONS
{
  /* ELF header + program headers live at 0x40000000, in the first PT_LOAD.
     Code follows right after them. */
  . = 0x40000000 + SIZEOF_HEADERS;

  /* ---------------- Code (R+X) ---------------- */
  .init : { KEEP(*(.init)) }
  .plt  : { *(.plt .plt.*) *(.iplt) }
  .text : {
    *(.text.unlikely .text.*_unlikely .text.unlikely.*)
    *(.text.exit .text.exit.*)
    *(.text.startup .text.startup.*)
    *(.text.hot .text.hot.*)
    *(.text .stub .text.*)
  }
  .fini : { KEEP(*(.fini)) }

  /* ---------------- Read-only data (R) ---------------- */
  . = ALIGN(0x1000);
  .rodata : { *(.rodata .rodata.*) }
  .note.gnu.property : { *(.note.gnu.property) }

  .eh_frame_hdr : { *(.eh_frame_hdr) *(.eh_frame_entry .eh_frame_entry.*) }
  .eh_frame     : ONLY_IF_RO { KEEP(*(.eh_frame)) *(.eh_frame.*) }
  .gcc_except_table : ONLY_IF_RO { *(.gcc_except_table .gcc_except_table.*) }

  /* ---------------- Writable data (R+W) ---------------- */
  . = ALIGN(0x1000);

  /* TLS template. musl reads PT_TLS from the program headers. */
  .tdata : {
    PROVIDE_HIDDEN(__tdata_start = .);
    *(.tdata .tdata.* .gnu.linkonce.td.*)
  }
  .tbss : {
    *(.tbss .tbss.* .gnu.linkonce.tb.*)
    *(.tcommon)
  }

  .preinit_array : {
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array))
    PROVIDE_HIDDEN(__preinit_array_end = .);
  }
  .init_array : {
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
    KEEP(*(.init_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .ctors))
    PROVIDE_HIDDEN(__init_array_end = .);
  }
  .fini_array : {
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
    KEEP(*(.fini_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .dtors))
    PROVIDE_HIDDEN(__fini_array_end = .);
  }

  .data.rel.ro : { *(.data.rel.ro .data.rel.ro.*) }
  .got     : { *(.got) *(.igot) }
  .got.plt : { *(.got.plt) *(.igot.plt) }

  .eh_frame         : ONLY_IF_RW { KEEP(*(.eh_frame)) *(.eh_frame.*) }
  .gcc_except_table : ONLY_IF_RW { *(.gcc_except_table .gcc_except_table.*) }

  .data : { *(.data .data.* .gnu.linkonce.d.*) }

  PROVIDE(__bss_start = .);
  .bss : {
    *(.dynbss)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN(8);
  }
  . = ALIGN(0x1000);
  PROVIDE(_end = .);
  PROVIDE(end = .);
  PROVIDE(__end = .);

  /* ---------------- Debug info (not loaded) ---------------- */
  .comment 0 : { *(.comment) }
  .debug          0 : { *(.debug) }
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_ranges   0 : { *(.debug_ranges) }
  .debug_line_str 0 : { *(.debug_line_str) }
  .debug_rnglists 0 : { *(.debug_rnglists) }
  .debug_loclists 0 : { *(.debug_loclists) }
  .debug_macro    0 : { *(.debug_macro) }
  .symtab         0 : { *(.symtab) }
  .strtab         0 : { *(.strtab) }
  .shstrtab       0 : { *(.shstrtab) }

  /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) }
}