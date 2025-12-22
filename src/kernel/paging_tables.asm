; paging_tables.asm

[BITS 32]
global pml4_table

section .data
align 4096
pml4_table:
    dq pdpt + 0x003            ; P + RW, user=0 (flags 0x3)
    times 511 dq 0

align 4096
pdpt:
    dq 0x0000000000000083      ; 1 GiB page: base=0, PS=1, RW=1, P=1
    times 511 dq 0

; [BITS 32]
; global pml4_table

; section .data
; align 4096
; pml4_table:
;     dq pdpt + 0x003
;     times 511 dq 0

; align 4096
; pdpt:
;     dq 0x0000000000000083
;     times 511 dq 0
