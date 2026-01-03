.section .text.crt0,"ax"
.globl _start
.extern main

_start:
    call   main            # int main(void)
    mov    %rax, %rdi      # exit code
    mov    $2, %rax        # SYS_exit = 2
    xor    %rsi, %rsi
    xor    %rdx, %rdx
    syscall

1:  jmp 1b                 # if syscall returns, spin forever
