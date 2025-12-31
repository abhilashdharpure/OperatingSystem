; user_syscall.asm
bits 64
global do_syscall

section .text
do_syscall:
    mov rax, rdi    ; syscall number
    mov rdi, rsi    ; arg1
    mov rsi, rdx    ; arg2
    mov rdx, rcx    ; arg3
    int 0x80
    ret
