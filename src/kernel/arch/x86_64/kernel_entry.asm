; 64-bit low stub
SECTION .boot64_stub
[BITS 64]
global kernel_low_entry
; extern kernel_entry

kernel_low_entry:
    mov dx, 0x3F8
    mov al, '8'
    out dx, al

    mov     rax, kernel_entry

    mov dx, 0x3F8
    mov al, '9'
    out dx, al

    jmp     rax       ; kernel_entry is also in low identity-mapped region

; Kernel proper, low identity-mapped
SECTION .text
[BITS 64]
global kernel_entry
extern _kernel_stack_top
extern early_kernel_main

kernel_entry:
    cli
    mov dx, 0x3F8
    mov al, 'A'
    out dx, al

    mov     rsp, _kernel_stack_top
    and     rsp, -16
    xor     rbp, rbp

    mov dx, 0x3F8
    mov al, 'B'
    out dx, al

    ; RDI still has multiboot info
    call    early_kernel_main

.hang:
    hlt
    jmp .hang
