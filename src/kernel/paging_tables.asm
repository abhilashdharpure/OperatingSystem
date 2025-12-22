; paging_tables.asm

[BITS 32]
global pml4_table

section .data
align 4096
pml4_table:
    dq pdpt + 0x003
    times 511 dq 0

align 4096
pdpt:
    dq 0x0000000000000083
    times 511 dq 0

; [BITS 64]

; global pml4_table

; section .data
; align 4096
; pml4_table:
;     dq pdpt + 0x003      ; Present | RW
;     times 511 dq 0

; align 4096
; pdpt:
;     dq 0x0000000000000083   ; Present | RW | PS (1 GiB page)
;     times 511 dq 0


; [BITS 32]

; global pml4_table

; section .data
; align 4096
; pml4_table:
;     dq pdpt + 0x003          ; present + writable

; align 4096
; pdpt:
;     ; 1 GiB identity map: 0x00000000..0x3FFFFFFF → same physical
;     ; 0x083 = present (1) + writable (2) + PS (bit 7 = 0x80)
;     dq 0x0000000000000083    ; 1 GiB page at phys 0

;     ; Remaining PDPT entries unused for now
;     times 511 dq 0
