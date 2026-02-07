; syscall64_asm.asm
[BITS 64]

global x64_syscall_entry

extern syscall_dispatch
extern g_syscall_rsp0

section .text

x64_syscall_entry:
    swapgs

    mov     r10, rcx          ; user RIP
    mov     r11, r11          ; user RFLAGS
    mov     r9,  rsp          ; user RSP

    mov     rsp, [rel g_syscall_rsp0]

    push    rbx
    push    rbp
    push    r12
    push    r13
    push    r14
    push    r15
    push    r10             ; user RIP
    push    r11             ; user RFLAGS
    push    r9              ; user RSP

    mov     r15, r9         ; stash a5
    mov     r9,  r8
    mov     r8,  r10
    mov     rcx, rdx
    mov     rdx, rsi
    mov     rsi, rdi
    mov     rdi, rax

    push    r15
    call    syscall_dispatch
    add     rsp, 8

    pop     r9              ; user RSP
    pop     r11             ; user RFLAGS
    pop     r10             ; user RIP
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbp
    pop     rbx

    push    qword 0x23
    push    r9
    push    r11
    push    qword 0x1B
    push    r10

    swapgs
    iretq




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
