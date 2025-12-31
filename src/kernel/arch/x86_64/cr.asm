BITS 64
GLOBAL write_cr3

SECTION .text
write_cr3:
    mov     rax, rdi      ; SysV ABI: first arg in RDI
    mov     cr3, rax
    ret
