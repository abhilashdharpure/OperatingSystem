; syscall64_asm.asm
[BITS 64]

global x64_syscall_entry

extern syscall_dispatch
extern g_syscall_rsp0

section .text

; On entry from SYSCALL:
;   rax = nr
;   rdi = a0
;   rsi = a1
;   rdx = a2
;   r10 = a3
;   r8  = a4
;   r9  = a5
;   rcx = user RIP
;   r11 = user RFLAGS
;   rsp = user RSP
;
; We want to call:
;   uint64_t syscall_dispatch(uint64_t nr,
;                             uint64_t a0,
;                             uint64_t a1,
;                             uint64_t a2,
;                             uint64_t a3,
;                             uint64_t a4,
;                             uint64_t a5);

x64_syscall_entry:
    swapgs

    ; Save user RSP in a callee-saved register
    mov     r12, rsp              ; r12 = user RSP

    ; Switch to kernel stack
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

    ; Save user RIP and RFLAGS in callee-saved regs
    mov     r13, rcx              ; r13 = user RIP
    mov     r14, r11              ; r14 = user RFLAGS

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

    ; Restore user RIP/RFLAGS
    mov     rcx, r13              ; user RIP
    mov     r11, r14              ; user RFLAGS

    ; Restore saved volatile regs
    pop     r10
    pop     r9
    pop     r8

    ; Restore callee-saved regs
    pop     r15
    pop     r14
    pop     r13
    pop     r12
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

; x64_syscall_entry:
;     swapgs

;     ; Save user RSP and a3 before we touch r10
;     mov     r15, r10          ; r15 = a3
;     mov     r10, rsp          ; r10 = user RSP

;     ; Switch to kernel stack
;     mov     rsp, [rel g_syscall_rsp0]

;     ; Save all regs we must preserve across syscall
;     push    rbx
;     push    rbp
;     push    r12
;     push    r13
;     push    r14
;     push    r15
;     push    r8
;     push    r9
;     push    r10

;     ; Save user context (RIP, RFLAGS, RSP)
;     push    rcx               ; user RIP
;     push    r11               ; user RFLAGS
;     push    r10               ; user RSP

;     ; Now set up args for syscall_dispatch
;     ; rax = nr, rdi=a0, rsi=a1, rdx=a2, r15=a3, r8=a4, r9=a5

;     mov     rcx, rdx          ; rcx = a2
;     mov     rdx, rsi          ; rdx = a1
;     mov     rsi, rdi          ; rsi = a0
;     mov     rdi, rax          ; rdi = nr

;     ; r15 = a3, r8 = a4, r9 = a5
;     mov     r10, r9           ; r10 = a5 (stash)
;     mov     r9,  r8           ; r9  = a4
;     mov     r8,  r15          ; r8  = a3

;     push    r10               ; a5 as 7th argument
;     call    syscall_dispatch
;     add     rsp, 8

;     ; Restore user context
;     pop     r10               ; user RSP
;     pop     r11               ; user RFLAGS
;     pop     rcx               ; user RIP

;     ; Restore preserved regs in reverse order
;     pop     r10
;     pop     r9
;     pop     r8
;     pop     r15
;     pop     r14
;     pop     r13
;     pop     r12
;     pop     rbp
;     pop     rbx

;     ; Build iret frame
;     push    qword 0x23        ; user SS
;     push    r10               ; user RSP
;     push    r11               ; user RFLAGS
;     push    qword 0x1B        ; user CS
;     push    rcx               ; user RIP

;     swapgs
;     iretq




; [BITS 64]

; global x64_syscall_entry

; extern syscall_dispatch
; extern g_syscall_rsp0

; section .text

; x64_syscall_entry:
;     swapgs

;     mov     r10, rcx          ; user RIP
;     mov     r11, r11          ; user RFLAGS
;     mov     r9,  rsp          ; user RSP

;     mov     rsp, [rel g_syscall_rsp0]

;     push    rbx
;     push    rbp
;     push    r12
;     push    r13
;     push    r14
;     push    r15
;     push    r10             ; user RIP
;     push    r11             ; user RFLAGS
;     push    r9              ; user RSP

;     mov     r15, r9         ; stash a5
;     mov     r9,  r8
;     mov     r8,  r10
;     mov     rcx, rdx
;     mov     rdx, rsi
;     mov     rsi, rdi
;     mov     rdi, rax

;     push    r15
;     call    syscall_dispatch
;     add     rsp, 8

;     pop     r9              ; user RSP
;     pop     r11             ; user RFLAGS
;     pop     r10             ; user RIP
;     pop     r15
;     pop     r14
;     pop     r13
;     pop     r12
;     pop     rbp
;     pop     rbx

;     push    qword 0x23
;     push    r9
;     push    r11
;     push    qword 0x1B
;     push    r10

;     swapgs
;     iretq


; [BITS 64]

; global x64_syscall_entry

; extern syscall_dispatch
; extern g_syscall_rsp0

; section .text

; x64_syscall_entry:
;     swapgs

;     mov     r10, rcx          ; user RIP
;     mov     r11, r11          ; user RFLAGS
;     mov     r9,  rsp          ; user RSP

;     mov     rsp, [rel g_syscall_rsp0]

;     push    rbx
;     push    rbp
;     push    r12
;     push    r13
;     push    r14
;     push    r15
;     push    r10             ; user RIP
;     push    r11             ; user RFLAGS
;     push    r9              ; user RSP

;     mov     r15, r9         ; stash a5
;     mov     r9,  r8
;     mov     r8,  r10
;     mov     rcx, rdx
;     mov     rdx, rsi
;     mov     rsi, rdi
;     mov     rdi, rax

;     push    r15
;     call    syscall_dispatch
;     add     rsp, 8

;     pop     r9              ; user RSP
;     pop     r11             ; user RFLAGS
;     pop     r10             ; user RIP
;     pop     r15
;     pop     r14
;     pop     r13
;     pop     r12
;     pop     rbp
;     pop     rbx

;     push    qword 0x23
;     push    r9
;     push    r11
;     push    qword 0x1B
;     push    r10

;     swapgs
;     iretq
