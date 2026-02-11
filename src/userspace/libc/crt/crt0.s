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



; .global _start
; _start:
;     // System V AMD64 ABI: argc in %rdi, argv in %rsi, envp in %rdx if you want
;     // For now, just call main() with no args.

;     call main

;     // main() returns int in %eax
;     mov %eax, %edi        // exit code in %edi
;     mov $60, %eax         // SYS_exit on Linux; in your OS use your SYS_exit number
;     syscall

;     hlt                   // just in case
