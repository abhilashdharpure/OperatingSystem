.global _start
.extern main
.extern syscall_exit

_start:
    call main
    mov %eax, %ebx
    call syscall_exit
