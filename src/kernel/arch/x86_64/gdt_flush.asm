[BITS 64]
global gdt_flush

%define GDT_KERNEL_CODE 0x08
%define KERNEL_DATA_SELECTOR 0x10

section .text

; void gdt_flush(uint64_t gdtr_ptr)
; rdi = &GDTR
gdt_flush:
    lgdt [rdi]

    ; Reload data segments
    mov ax, KERNEL_DATA_SELECTOR
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Reload CS
    push qword GDT_KERNEL_CODE
    lea rax, [rel .reload]
    push rax
    retfq

.reload:
    ; DO NOT ret
    ; execution continues back to C
    nop
