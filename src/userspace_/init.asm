global _start

_start:
    mov eax, 1          ; SYS_WRITE
    mov ebx, 1          ; fd = stdout
    mov ecx, msg
    mov edx, msg_len
    int 0x80

hang:
    jmp hang

section .rodata
msg db "HELLO FROM USERSPACE", 10
msg_len equ $ - msg
