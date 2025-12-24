[BITS 64]
global gdt_reload

%define KERNEL_DATA_SELECTOR 0x10

section .text
; void gdt_reload(uint64_t gdtr_ptr)
; rdi = &GDTR
gdt_reload:
    lgdt [rdi]          ; load GDTR
    ret                  ; no need to reload segments for TSS install
