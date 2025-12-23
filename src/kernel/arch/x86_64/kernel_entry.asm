;kernel_entry.asm

[BITS 64]
global kernel_entry
extern _kernel_stack_top
extern early_kernel_main

kernel_entry:
    cli

    ; mov dx, 0x3F8
    ; mov al, 'A'
    ; out dx, al

    mov rsp, _kernel_stack_top
    and rsp, -16
    xor rbp, rbp

    ; mov al, 'X'
    ; out dx, al

    ; mov al, 'B'
    ; out dx, al

    ; mov al, 'C'
    ; out dx, al

    call early_kernel_main

.hang:
    hlt
    jmp .hang