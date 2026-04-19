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

; x64_syscall_entry:
;     swapgs

;     ; Save user RSP
;     mov     r12, rsp

;     ; Switch to kernel stack
;     mov     rsp, [rel g_syscall_rsp0]

;     ; ---- Save callee-saved ----
;     push    rbx
;     push    rbp
;     push    r12
;     push    r13
;     push    r14
;     push    r15          ; 6 pushes = 48 bytes

;     ; ---- Save volatile we need ----
;     push    r8
;     push    r9           ; 2 pushes = 16 bytes, total 64

;     ; ---- Save syscall state ----
;     push    rcx          ; user RIP
;     push    r11          ; user RFLAGS
;                          ; 2 pushes = 16 bytes, total 80

;     ; At this point, we have pushed 10 regs = 80 bytes.
;     ; RSP is 16-byte aligned (assuming g_syscall_rsp0 was).

;     ; ---- Preserve args ----
;     mov     r15, r10     ; a3
;     mov     rbx, r8      ; a4
;     mov     rbp, r9      ; a5

;     ; ---- Reorder args for SysV ABI ----
;     mov     rcx, rdx     ; a2
;     mov     rdx, rsi     ; a1
;     mov     rsi, rdi     ; a0
;     mov     rdi, rax     ; nr

;     mov     r8,  r15     ; a3
;     mov     r9,  rbx     ; a4

;     ; ---- Stack for 7th arg + alignment ----
;     ; We currently have 80 bytes pushed.
;     ; SysV requires: (RSP + 8) % 16 == 0 at function entry.
;     ; So before 'call', RSP must be 16-byte aligned.
;     ;
;     ; Plan:
;     ;   sub rsp, 8   -> 88 bytes (misaligned, 8 mod 16)
;     ;   push a5      -> 96 bytes (aligned, 0 mod 16)
;     ;   call         -> pushes return address, RSP -= 8
;     ;                   => 104 bytes total pushed, so at
;     ;                   syscall_dispatch entry, RSP % 16 == 8 (correct).

;     sub     rsp, 8               ; alignment padding
;     push    rbp                  ; a5 (7th arg) at [rsp+8] in callee

;     call    syscall_dispatch

;     ; After return:
;     ;   RSP points to a5 (we pushed it), with the padding 8 bytes above.
;     add     rsp, 16              ; drop a5 + padding

;     ; ---- Restore syscall state ----
;     pop     r11                  ; user RFLAGS
;     pop     rcx                  ; user RIP

;     ; Ensure IF flag set
;     or      r11, (1 << 9)

;     ; ---- Restore volatile ----
;     pop     r9
;     pop     r8

;     ; ---- Restore callee-saved ----
;     pop     r15
;     pop     r14
;     pop     r13
;     pop     r12                  ; user RSP
;     pop     rbp
;     pop     rbx

;     ; ---- Build iret frame ----
;     push    qword 0x23           ; SS (user data)
;     push    r12                  ; RSP
;     push    r11                  ; RFLAGS
;     push    qword 0x1B           ; CS (user code)
;     push    rcx                  ; RIP

;     swapgs
;     iretq
