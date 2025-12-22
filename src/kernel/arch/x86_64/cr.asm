BITS 64
GLOBAL write_cr3

SECTION .text
write_cr3:
    mov cr3, rdi   ; SysV ABI: first arg in RDI
    ret
