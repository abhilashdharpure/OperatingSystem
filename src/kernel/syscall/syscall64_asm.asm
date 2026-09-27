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





; [BITS 64]

; global x64_syscall_entry

; extern syscall_dispatch
; extern g_syscall_rsp0

; section .text

; ; SYSCALL entry:
; ;   RCX = user RIP
; ;   R11 = user RFLAGS
; ;   RSP = TSS.rsp0 (kernel stack)
; ;   CS  = kernel CS
; ;   SS  = kernel SS

; x64_syscall_entry:
;     swapgs
;     mov     r12, rsp                  ; save user RSP
;     mov     rsp, [rel g_syscall_rsp0] ; switch to kernel syscall stack

;     ; save callee-saved
;     push    rbx
;     push    rbp
;     push    r12
;     push    r13
;     push    r14
;     push    r15

;     ; save user RIP/RFLAGS
;     push    rcx
;     push    r11

;     ; ---------------------------------
;     ; preserve original arg registers
;     ; ---------------------------------
;     mov     r15, rdi        ; orig a0
;     mov     r14, rsi        ; orig a1
;     mov     r13, rdx        ; orig a2
;     mov     rbx, r10        ; orig a3
;     mov     rbp, r8         ; orig a4
;     mov     r12, r9         ; orig a5

;     ; reorder for syscall_dispatch(nr,a0..a5)
;     mov     rdi, rax        ; nr
;     mov     rsi, r15        ; a0
;     mov     rdx, r14        ; a1
;     mov     rcx, r13        ; a2
;     mov     r8,  rbx        ; a3
;     mov     r9,  rbp        ; a4
;     push    r12             ; a5

;     call    syscall_dispatch
;     add     rsp, 8          ; pop a5

;     ; restore user RFLAGS/RIP
;     pop     r11
;     pop     rcx

;     ; restore callee-saved
;     pop     r15
;     pop     r14
;     pop     r13
;     pop     r12             ; user RSP
;     pop     rbp
;     pop     rbx

;     ; build IRET frame
;     push    qword 0x23      ; user SS
;     push    r12             ; user RSP
;     push    r11             ; user RFLAGS
;     push    qword 0x1B      ; user CS
;     push    rcx             ; user RIP

;     swapgs
;     iretq





; ; syscall64_asm.asm
; [BITS 64]

; global x64_syscall_entry

; extern syscall_dispatch
; extern g_syscall_rsp0

; section .text

; x64_syscall_entry:
;     swapgs

;     ; Save user RSP before switching to kernel stack
;     mov     r12, rsp              ; r12 = user RSP

;     ; Switch to kernel syscall stack
;     mov     rsp, [rel g_syscall_rsp0]

;     ; Save callee-saved regs
;     push    rbx
;     push    rbp
;     push    r12
;     push    r13
;     push    r14
;     push    r15

;     ; Save volatile regs we want to restore later
;     push    r8
;     push    r9
;     push    r10

;     ; Save user RIP/RFLAGS exactly as given by SYSCALL
;     ;   rcx = user RIP
;     ;   r11 = user RFLAGS
;     push    rcx                   ; save user RIP
;     push    r11                   ; save user RFLAGS

;     ; Preserve a3, a4, a5 in temps
;     mov     r15, r10              ; r15 = a3
;     mov     rbx, r8               ; rbx = a4
;     mov     rbp, r9               ; rbp = a5

;     ; Reorder for C ABI:
;     ; syscall_dispatch(nr, a0, a1, a2, a3, a4, a5)
;     mov     rcx, rdx              ; rcx = a2
;     mov     rdx, rsi              ; rdx = a1
;     mov     rsi, rdi              ; rsi = a0
;     mov     rdi, rax              ; rdi = nr

;     mov     r8,  r15              ; r8  = a3
;     mov     r9,  rbx              ; r9  = a4

;     ; 7th arg (a5) on stack
;     push    rbp                   ; a5

;     call    syscall_dispatch
;     add     rsp, 8                ; pop a5

;     ; Restore user RFLAGS and RIP from stack
;     pop     r11                   ; user RFLAGS
;     pop     rcx                   ; user RIP

;     ; Restore saved volatile regs
;     pop     r10
;     pop     r9
;     pop     r8

;     ; Restore callee-saved regs
;     pop     r15
;     pop     r14
;     pop     r13
;     pop     r12                   ; r12 = user RSP
;     pop     rbp
;     pop     rbx

;     ; Build iret frame with the *real* user context
;     push    qword 0x23            ; user SS
;     push    r12                   ; user RSP
;     push    r11                   ; user RFLAGS
;     push    qword 0x1B            ; user CS
;     push    rcx                   ; user RIP

;     swapgs
;     iretq






