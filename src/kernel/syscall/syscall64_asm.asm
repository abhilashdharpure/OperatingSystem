; syscall64.asm
[BITS 64]
global x64_syscall_entry
extern syscall_dispatch

section .text

x64_syscall_entry:
    swapgs

    ; On entry:
    ;   rax = syscall number
    ;   rdi = a0
    ;   rsi = a1
    ;   rdx = a2
    ;   rcx = user RIP
    ;   r11 = user RFLAGS
    ;   rsp = user RSP
    ;   r10 = a3
    ;   r8  = a4
    ;   r9  = a5

    mov     r13, rsp        ; save user RSP for IRET frame

    ; Build IRETQ frame on kernel stack
    push    qword 0x23      ; SS (user data)
    push    r13             ; RSP (original user RSP)
    push    r11             ; RFLAGS
    push    qword 0x1B      ; CS (user code)
    push    rcx             ; RIP (original user RIP)

    ; Set up SysV call:
    ;   syscall_dispatch(a0, a1, a2, a3, a4, a5)
    ;
    ; rdi = a0
    ; rsi = a1
    ; rdx = a2
    ; r10 = a3
    ; r8  = a4
    ; r9  = a5

    mov     rcx, r10        ; rcx = a3
    ; rdi, rsi, rdx, r8, r9 already correct
    ; rax still holds nr

    call    syscall_dispatch

    swapgs
    iretq


; ; syscall64.asm
; [BITS 64]
; global x64_syscall_entry
; extern syscall_dispatch

; section .text

; x64_syscall_entry:
;     swapgs

;     ; On entry:
;     ;   rax = syscall number
;     ;   rdi = arg0
;     ;   rsi = arg1
;     ;   rdx = arg2
;     ;   rcx = user RIP
;     ;   r11 = user RFLAGS
;     ;   rsp = kernel RSP
;     ;   r10 = arg3
;     ;   r8  = arg4
;     ;   r9  = arg5

;     mov     r12, rsp        ; save kernel RSP (optional, for debugging)

;     ; Build IRETQ frame on kernel stack
;     push    qword 0x23      ; SS (user data)
;     push    qword 0         ; RSP (user RSP, fill later if you track it)
;     push    r11             ; RFLAGS
;     push    qword 0x1B      ; CS (user code)
;     push    rcx             ; RIP (user RIP)

;     ; Now set up C call:
;     ;   syscall_dispatch(a0, a1, a2, a3, a4, a5)
;     ;
;     ; Current regs:
;     ;   rdi = a0
;     ;   rsi = a1
;     ;   rdx = a2
;     ;   r10 = a3
;     ;   r8  = a4
;     ;   r9  = a5
;     ;
;     ; SysV ABI wants:
;     ;   rdi = a0
;     ;   rsi = a1
;     ;   rdx = a2
;     ;   rcx = a3
;     ;   r8  = a4
;     ;   r9  = a5

;     mov     rcx, r10        ; rcx = a3

;     ; rdi, rsi, rdx, r8, r9 already correct

;     call    syscall_dispatch

;     swapgs
;     iretq
