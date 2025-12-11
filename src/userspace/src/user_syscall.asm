; user_syscall.asm
bits 32
global do_syscall

section .text

do_syscall:
    mov eax, [esp+4]   ; syscall number
    mov ebx, [esp+8]   ; arg1
    mov ecx, [esp+12]  ; arg2
    mov edx, [esp+16]  ; arg3
    int 0x80
    ret
