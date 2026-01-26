[BITS 64]
global x64_syscall_entry
extern syscall_dispatch
extern g_syscall_rsp0

section .text

; MSR layout on syscall entry:
;   rax = syscall number
;   rdi = arg0
;   rsi = arg1
;   rdx = arg2
;   r10 = arg3
;   r8  = arg4
;   r9  = arg5
;   rcx = user RIP
;   r11 = user RFLAGS
;   rsp = user RSP

x64_syscall_entry:
    swapgs

    ; capture user context from SYSCALL
    mov     r12, rcx        ; user RIP
    mov     r13, r11        ; user RFLAGS
    mov     r14, rsp        ; user RSP

    ; stash syscall args in callee-saved regs
    mov     rbp, rdi        ; a0
    mov     rbx, rsi        ; a1
    ; rdx = a2, r10 = a3, r8 = a4, r9 = a5, rax = nr

    ; switch to kernel stack
    mov     rsp, [rel g_syscall_rsp0]

    ; save callee-saved for C (SysV: rbx, rbp, r12–r15)
    push    rbx
    push    rbp
    push    r12
    push    r13
    push    r14
    push    r15

    ; set up args for:
    ;   uint64_t syscall_dispatch(uint64_t a0,
    ;                             uint64_t a1,
    ;                             uint64_t a2,
    ;                             uint64_t a3,
    ;                             uint64_t a4,
    ;                             uint64_t a5);
    mov     rdi, rbp        ; a0
    mov     rsi, rbx        ; a1
    ; rdx = a2
    mov     rcx, r10        ; a3
    ; r8  = a4
    ; r9  = a5

    call    syscall_dispatch    ; rax = return value to user

    ; restore callee-saved
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbp
    pop     rbx

    ; now build IRET frame from the saved user context:
    ;   RIP, CS, RFLAGS, RSP, SS
    push    qword 0x23      ; SS (user data, DPL=3)
    push    r14             ; RSP (user)
    push    r13             ; RFLAGS (user)
    push    qword 0x1B      ; CS (user code, DPL=3)
    push    r12             ; RIP (user)

    swapgs
    iretq
