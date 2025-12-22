; paging_asm.asm
[bits 64]

global enter_user_mode
enter_user_mode:
    ; rdi = user_ds, rsi = user_esp, rdx = user_eflags, rcx = user_cs, r8 = user_eip
    cli
    push rdi       ; SS
    push rsi       ; ESP
    push rdx       ; EFLAGS
    push rcx       ; CS
    push r8        ; EIP
    iretq

; global pml4_table
; section .bss
; align 4096
; pml4_table:
;     ; 512 entries * 8 bytes = 4096 bytes
;     resq 512