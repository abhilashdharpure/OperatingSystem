section .multiboot_header
align 8

MB2_MAGIC      equ 0xE85250D6
MB2_ARCH_I386  equ 0               ; 32-bit
MB2_HEADER_LEN equ header_end - header_start

header_start:
    dd MB2_MAGIC
    dd MB2_ARCH_I386
    dd MB2_HEADER_LEN
    dd -(MB2_MAGIC + MB2_ARCH_I386 + MB2_HEADER_LEN)

    ; End tag
    dw 0
    dw 0
    dd 8
header_end:
