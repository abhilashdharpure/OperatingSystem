;syscall64_asm.asm

[BITS 64]

global x64_syscall_entry

extern syscall_dispatch
extern g_syscall_rsp0
extern klog_asm
extern syscall_next_rip

section .text

msg_syscall_entry:
    db "[ASM] x64_syscall_entry reached", 10, 0

; On syscall entry (x86_64):
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

    ; capture user context
    mov     r12, rcx        ; user RIP
    mov     r13, r11        ; user RFLAGS
    mov     r14, rsp        ; user RSP

    ; save syscall args on current (user) stack
    push    rdi             ; a0
    push    rsi             ; a1
    push    rdx             ; a2
    push    r10             ; a3
    push    r8              ; a4
    push    r9              ; a5
    push    rax             ; nr

    ; log (clobbers caller-saved regs, but we saved what we need)
    mov     rdi, msg_syscall_entry
    call    klog_asm

    ; restore syscall regs
    pop     rax             ; nr
    pop     r9              ; a5
    pop     r8              ; a4
    pop     r10             ; a3
    pop     rdx             ; a2
    pop     rsi             ; a1
    pop     rdi             ; a0

    ; switch to kernel stack
    mov     rsp, [rel g_syscall_rsp0]

    ; stash args in callee-saved regs for C
    mov     rbp, rdi        ; a0
    mov     rbx, rsi        ; a1
    ; rdx = a2
    mov     rcx, r10        ; a3
    ; r8  = a4
    ; r9  = a5

    ; save callee-saved for C (but NOT r12–r14: they hold user context)
    push    rbx
    push    rbp
    push    r15

    ; call C dispatcher: uint64_t syscall_dispatch(...)
    call    syscall_dispatch

    ; restore callee-saved
    pop     r15
    pop     rbp
    pop     rbx

    ; optional RIP override from C (e.g. after SYS_set_tid_address)
    mov     rax, [rel syscall_next_rip]
    test    rax, rax
    jz      .no_override
    mov     r12, rax
    mov     qword [rel syscall_next_rip], 0
.no_override:

    ; r12/r13/r14 now final user RIP/RFLAGS/RSP
    push    qword 0x23      ; SS (user data)
    push    r14             ; RSP (user)
    push    r13             ; RFLAGS (user)
    push    qword 0x1B      ; CS (user code)
    push    r12             ; RIP (user)

    swapgs
    iretq