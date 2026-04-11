; syscall64_asm.asm
[BITS 64]

global x64_syscall_entry

extern syscall_dispatch
extern g_syscall_rsp0

section .text

x64_syscall_entry:
    swapgs

    ; Save user RSP before switching to kernel stack
    mov     r12, rsp              ; r12 = user RSP

    ; Switch to kernel syscall stack
    mov     rsp, [rel g_syscall_rsp0]

    ; Save callee-saved regs
    push    rbx
    push    rbp
    push    r12
    push    r13
    push    r14
    push    r15

    ; Save volatile regs we want to restore later
    push    r8
    push    r9
    push    r10

    ; Save user RIP/RFLAGS exactly as given by SYSCALL
    ;   rcx = user RIP
    ;   r11 = user RFLAGS
    push    rcx                   ; save user RIP
    push    r11                   ; save user RFLAGS

    ; Preserve a3, a4, a5 in temps
    mov     r15, r10              ; r15 = a3
    mov     rbx, r8               ; rbx = a4
    mov     rbp, r9               ; rbp = a5

    ; Reorder for C ABI:
    ; syscall_dispatch(nr, a0, a1, a2, a3, a4, a5)
    mov     rcx, rdx              ; rcx = a2
    mov     rdx, rsi              ; rdx = a1
    mov     rsi, rdi              ; rsi = a0
    mov     rdi, rax              ; rdi = nr

    mov     r8,  r15              ; r8  = a3
    mov     r9,  rbx              ; r9  = a4

    ; 7th arg (a5) on stack
    push    rbp                   ; a5

    call    syscall_dispatch
    add     rsp, 8                ; pop a5

    ; Restore user RFLAGS and RIP from stack
    pop     r11                   ; user RFLAGS
    pop     rcx                   ; user RIP

    ; Restore saved volatile regs
    pop     r10
    pop     r9
    pop     r8

    ; Restore callee-saved regs
    pop     r15
    pop     r14
    pop     r13
    pop     r12                   ; r12 = user RSP
    pop     rbp
    pop     rbx

    ; Build iret frame with the *real* user context
    push    qword 0x23            ; user SS
    push    r12                   ; user RSP
    push    r11                   ; user RFLAGS
    push    qword 0x1B            ; user CS
    push    rcx                   ; user RIP

    swapgs
    iretq




; ; syscall64_asm.asm
; [BITS 64]

; global x64_syscall_entry

; extern syscall_dispatch
; extern g_syscall_rsp0

; section .text

; ; SYSCALL calling convention (Linux x86-64):
; ;   rax = syscall number
; ;   rdi = arg0
; ;   rsi = arg1
; ;   rdx = arg2
; ;   r10 = arg3
; ;   r8  = arg4
; ;   r9  = arg5
; ;
; ; On entry:
; ;   rcx = user RIP
; ;   r11 = user RFLAGS
; ;
; ; We:
; ;   - switch to kernel stack from TSS.rsp0
; ;   - save user RSP
; ;   - reorder args to System V C ABI:
; ;       syscall_dispatch(nr, a0, a1, a2, a3, a4, a5)
; ;   - call syscall_dispatch
; ;   - restore user context and iretq

; x64_syscall_entry:
;     ; switch GS to kernel
;     swapgs

;     ; save user RSP before switching to kernel stack
;     mov     r12, rsp              ; r12 = user RSP

;     ; switch to kernel syscall stack (TSS.rsp0)
;     mov     rsp, [rel g_syscall_rsp0]

;     ; -------------------------
;     ; save callee-saved registers
;     ; -------------------------
;     push    rbx
;     push    rbp
;     push    r12
;     push    r13
;     push    r14
;     push    r15

;     ; -------------------------
;     ; save volatile regs we want back
;     ; -------------------------
;     push    r8
;     push    r9
;     push    r10

;     ; -------------------------
;     ; save user RIP/RFLAGS as given by SYSCALL
;     ;   rcx = user RIP
;     ;   r11 = user RFLAGS
;     ; -------------------------
;     push    rcx                   ; user RIP
;     push    r11                   ; user RFLAGS

;     ; -------------------------
;     ; preserve a3, a4, a5 in temps
;     ;   a3 = r10
;     ;   a4 = r8
;     ;   a5 = r9
;     ; -------------------------
;     mov     r15, r10              ; r15 = a3
;     mov     rbx, r8               ; rbx = a4
;     mov     rbp, r9               ; rbp = a5

;     ; -------------------------
;     ; reorder for System V C ABI:
;     ;   syscall_dispatch(nr, a0, a1, a2, a3, a4, a5)
;     ;
;     ; SYSCALL:
;     ;   rax = nr
;     ;   rdi = a0
;     ;   rsi = a1
;     ;   rdx = a2
;     ;   r10 = a3
;     ;   r8  = a4
;     ;   r9  = a5
;     ;
;     ; C ABI:
;     ;   rdi, rsi, rdx, rcx, r8, r9, then stack
;     ; -------------------------
;     mov     rcx, rdx              ; rcx = a2
;     mov     rdx, rsi              ; rdx = a1
;     mov     rsi, rdi              ; rsi = a0
;     mov     rdi, rax              ; rdi = nr

;     mov     r8,  r15              ; r8  = a3
;     mov     r9,  rbx              ; r9  = a4

;     ; 7th arg (a5) on stack
;     push    rbp                   ; a5

;     ; -------------------------
;     ; call C dispatcher
;     ; -------------------------
;     call    syscall_dispatch

;     ; drop a5 from stack
;     add     rsp, 8

;     ; -------------------------
;     ; restore user RFLAGS and RIP
;     ; -------------------------
;     pop     r11                   ; user RFLAGS
;     pop     rcx                   ; user RIP

;     ; -------------------------
;     ; restore saved volatile regs
;     ; -------------------------
;     pop     r10
;     pop     r9
;     pop     r8

;     ; -------------------------
;     ; restore callee-saved regs
;     ; -------------------------
;     pop     r15
;     pop     r14
;     pop     r13
;     pop     r12                   ; r12 = user RSP
;     pop     rbp
;     pop     rbx

;     ; -------------------------
;     ; build IRET frame for user
;     ; -------------------------
;     push    qword 0x23            ; user SS
;     push    r12                   ; user RSP
;     push    r11                   ; user RFLAGS
;     push    qword 0x1B            ; user CS
;     push    rcx                   ; user RIP

;     ; back to user GS
;     swapgs
;     iretq
