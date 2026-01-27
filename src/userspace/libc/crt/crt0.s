.section .text.crt0,"ax"
.globl _start
.extern main

_start:
    call   main
    mov    %rax, %rdi      # exit code
    mov    $2, %rax        # SYS_exit
    xor    %rsi, %rsi
    xor    %rdx, %rdx
    syscall
1:  jmp 1b
