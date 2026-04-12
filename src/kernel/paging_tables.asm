[BITS 64]
; global pml4_table

section .data
align 4096
; pml4_table:s
    ; dq pdpt + 0x003          ; Present | RW
    ; times 511 dq 0

align 4096
pdpt:
    dq 0x0000000000000083    ; Present | RW | PS (1 GiB page)
    times 511 dq 0
