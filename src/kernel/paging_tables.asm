; [BITS 64]
; ; global pml4_table

; section .data
; align 4096
; ; pml4_table:s
;     ; dq pdpt + 0x003          ; Present | RW
;     ; times 511 dq 0

; align 4096
; pdpt:
;     dq 0x0000000000000083    ; Present | RW | PS (1 GiB page)
;     times 511 dq 0


; arch/x86_64/paging_tables.asm
; 64-bit bootstrap paging: identity-map first 1 GiB using 2 MiB pages.
; PML4 -> PDPT -> PD (2 MiB PS entries). No 1 GiB huge page in PDPT.

BITS 64
default rel

global pml4_table          ; visible to C as `extern uint64_t pml4_table[]`

; Put this in the same low/stub data section you used before:
; previously: __attribute__((section(".data.boot")))
; or if your linker uses .boot64_stub_data for low data, use that instead.
section .data.boot align=4096

; -------------------------
; PML4
; -------------------------
pml4_table:
    dq pdpt + 0x003        ; Present | RW (user=0)
    times 511 dq 0

; -------------------------
; PDPT
; -------------------------
align 4096
pdpt:
    dq pd + 0x003          ; Present | RW, no PS bit here
    times 511 dq 0

; -------------------------
; PD – 2 MiB identity map for first 1 GiB
; -------------------------
align 4096
pd:
%assign i 0
%rep 512
    ; phys = i * 2MiB, flags = Present | RW | PS
    dq (i * 0x200000) + 0x083
%assign i i + 1
%endrep

section .note.GNU-stack noalloc noexec nowrite progbits


