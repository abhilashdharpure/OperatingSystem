;kernel_entry.asm

[BITS 64]
global kernel_entry
extern _kernel_stack_top
extern early_kernel_main

kernel_entry:
    cli

     ; The lower 32 bits of RDI already have the pointer
    mov rdi, rbx      ; move 64-bit register (rbx has lower 32-bit pointer, upper zeroed by default)

    mov rsp, _kernel_stack_top
    and rsp, -16
    xor rbp, rbp

    call early_kernel_main

.hang:
    hlt
    jmp .hang