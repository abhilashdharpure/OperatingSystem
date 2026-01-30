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

; .section .text.crt0,"ax"
; .globl _start
; .extern main

; _start:
;     call   main            # int main(void)
;     mov    %rax, %rdi      # exit code in rdi
;     mov    $60, %rax       # SYS_exit (Linux x86_64)
;     syscall
; 1:  jmp 1b
