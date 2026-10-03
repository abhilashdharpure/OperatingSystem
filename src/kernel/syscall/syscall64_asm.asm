[BITS 64]

global x64_syscall_entry

extern syscall_dispatch
extern g_syscall_rsp0
extern dbg_saved_rip
extern dbg_saved_rsp
extern dbg_saved_rflags

section .bss
align 8
g_user_rsp_tmp: resq 1

section .text

x64_syscall_entry:
    swapgs

    mov     [rel g_user_rsp_tmp], rsp
    mov     rsp, [rel g_syscall_rsp0]       ; must be 16-byte aligned

    ; ---- build iretq frame (SS, RSP, RFLAGS, CS, RIP) ----
    push    qword 0x23
    push    qword [rel g_user_rsp_tmp]
    push    r11                             ; user RFLAGS
    push    qword 0x1B
    push    rcx                             ; user RIP

    ; debug (r11 is already saved in the frame, so it is free as scratch)
    mov     r11, [rsp]
    mov     [rel dbg_saved_rip], r11
    mov     r11, [rsp + 16]
    mov     [rel dbg_saved_rflags], r11
    mov     r11, [rsp + 24]
    mov     [rel dbg_saved_rsp], r11

    ; ---- save every register the user expects preserved ----
    push    rbx
    push    rbp
    push    r12
    push    r13
    push    r14
    push    r15
    push    rdi
    push    rsi
    push    rdx
    push    r10
    push    r8
    push    r9

    ; Linux regs: rax=nr rdi=a0 rsi=a1 rdx=a2 r10=a3 r8=a4 r9=a5
    ; C ABI:      rdi=nr rsi=a0 rdx=a1 rcx=a2 r8=a3  r9=a4 [rsp]=a5
    ; 17 pushes so far = 136 bytes -> RSP is 8 mod 16; pushing a5 aligns it.
    push    r9              ; a5 (7th arg, on stack)
    mov     r9,  r8         ; a4
    mov     r8,  r10        ; a3
    mov     rcx, rdx        ; a2
    mov     rdx, rsi        ; a1
    mov     rsi, rdi        ; a0
    mov     rdi, rax        ; nr

    call    syscall_dispatch

    add     rsp, 8          ; drop a5

    ; rax = return value, do NOT touch it
    pop     r9
    pop     r8
    pop     r10
    pop     rdx
    pop     rsi
    pop     rdi
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbp
    pop     rbx

    swapgs
    iretq                   ; pops RIP, CS, RFLAGS, RSP, SS