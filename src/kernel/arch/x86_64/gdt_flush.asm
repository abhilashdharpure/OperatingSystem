; gdt_flush64.asm

[BITS 64]
global gdt_flush

%define GDT_KERNEL_CODE 0x08
%define KERNEL_DATA_SELECTOR 0x10

section .text

; void gdt_flush(uint64_t gdtr_ptr)
; rdi = &GDTR
gdt_flush:
    lgdt [rdi]

    ; Reload data segments (required in long mode)
    mov ax, KERNEL_DATA_SELECTOR
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Reload CS via far return
    push qword GDT_KERNEL_CODE
    lea rax, [rel .reload]
    push rax
    retfq              ; FAR return in 64-bit mode

.reload:
    ret


; BITS 64
; GLOBAL gdt_flush

; SECTION .text
; gdt_flush:
;     lgdt [rdi]        ; rdi points to GDTR structure

;     ; reload data segments
;     mov ax, 0x10      ; kernel data selector
;     mov ds, ax
;     mov es, ax
;     mov fs, ax
;     mov gs, ax
;     mov ss, ax

;     ; reload code segment
;     push 0x08         ; kernel code selector
;     lea rax, [rel .L1]
;     push rax
;     retfq

; .L1:                  ; return here after far jump
;     ret
