; gdt_asm.asm (x86_64 boot-only version)
[bits 32]

SECTION .data.boot
align 8

gdt64:
    dq 0x0000000000000000    ; null
    dq 0x00209A0000000000    ; 0x08: 64-bit code
    dq 0x0000920000000000    ; 0x10: data
gdt64_end:

global gdt64_desc_boot
gdt64_desc_boot:
    dw gdt64_end - gdt64 - 1 ; limit
    dd gdt64                 ; base low 32 bits
    dd 0                     ; base high 32 bits
