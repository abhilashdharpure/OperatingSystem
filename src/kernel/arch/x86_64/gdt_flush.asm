; gdt_flush64.asm
BITS 64
GLOBAL gdt_flush

SECTION .text
gdt_flush:
    lgdt [rdi]        ; rdi points to GDTR structure

    ; reload data segments
    mov ax, 0x10      ; kernel data selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; reload code segment
    push 0x08         ; kernel code selector
    lea rax, [rel .L1]
    push rax
    retfq

.L1:                  ; return here after far jump
    ret
