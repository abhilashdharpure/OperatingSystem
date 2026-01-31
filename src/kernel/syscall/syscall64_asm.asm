; syscall64_asm.asm
; 64-bit syscall entry for x86_64, using SYSCALL
; Assumes:
;   - LSTAR points to x64_syscall_entry
;   - STAR/FMASK set appropriately
;   - User code uses "syscall" (not int 0x80)
;
; Linux x86_64 syscall ABI:
;   On entry:
;     RAX = syscall number
;     RDI, RSI, RDX, R10, R8, R9 = args 0..5
;     RCX = user RIP
;     R11 = user RFLAGS
;     RSP = user RSP
;   On return:
;     RAX = return value
;     RCX, R11 may be clobbered
;     All callee-saved (RBX, RBP, R12–R15, RSP) must be preserved

[BITS 64]

global x64_syscall_entry

extern syscall_dispatch        ; long syscall_dispatch(long nr, ...);
extern g_syscall_rsp0          ; kernel stack top for syscalls

section .text

x64_syscall_entry:
    ; SYSCALL entry:
    ;   RCX = user RIP
    ;   R11 = user RFLAGS
    ;   RSP = user RSP
    ;   RAX = nr
    ;   RDI,RSI,RDX,R10,R8,R9 = args 0..5

    swapgs

    ; Save user RIP/RFLAGS/RSP into caller-saved temps
    mov     r10, rcx          ; user RIP
    mov     r11, r11          ; user RFLAGS (already there)
    mov     r9,  rsp          ; user RSP

    ; Switch to kernel syscall stack
    mov     rsp, [rel g_syscall_rsp0]

    ; Save user callee-saved regs and user context on kernel stack
    ; Layout (top of stack after pushes):
    ;   [rsp+0x00] user RBX
    ;   [rsp+0x08] user RBP
    ;   [rsp+0x10] user R12
    ;   [rsp+0x18] user R13
    ;   [rsp+0x20] user R14
    ;   [rsp+0x28] user R15
    ;   [rsp+0x30] user RIP
    ;   [rsp+0x38] user RFLAGS
    ;   [rsp+0x40] user RSP

    push    rbx
    push    rbp
    push    r12
    push    r13
    push    r14
    push    r15
    push    r10             ; user RIP
    push    r11             ; user RFLAGS
    push    r9              ; user RSP

    ; Linux syscall ABI on entry:
    ;   rax = nr
    ;   rdi = a0
    ;   rsi = a1
    ;   rdx = a2
    ;   r10 = a3
    ;   r8  = a4
    ;   r9  = a5
    ;
    ; SysV C ABI for syscall_dispatch:
    ;   rdi = nr
    ;   rsi = a0
    ;   rdx = a1
    ;   rcx = a2
    ;   r8  = a3
    ;   r9  = a4
    ;   a5  on stack

    mov     r15, r9         ; stash a5

    mov     r9,  r8         ; r9 = a4
    mov     r8,  r10        ; r8 = a3
    mov     rcx, rdx        ; rcx = a2
    mov     rdx, rsi        ; rdx = a1
    mov     rsi, rdi        ; rsi = a0
    mov     rdi, rax        ; rdi = nr

    push    r15             ; a5 as 7th arg

    call    syscall_dispatch

    add     rsp, 8          ; pop a5

    ; rax now holds return value

    ; Restore user context and callee-saved regs
    pop     r9              ; user RSP
    pop     r11             ; user RFLAGS
    pop     r10             ; user RIP
    pop     r15             ; user R15
    pop     r14             ; user R14
    pop     r13             ; user R13
    pop     r12             ; user R12
    pop     rbp             ; user RBP
    pop     rbx             ; user RBX

    ; Build IRET frame from restored user context
    ; 0x1B = user CS, 0x23 = user SS (RPL=3)
    push    qword 0x23      ; user SS
    push    r9              ; user RSP
    push    r11             ; user RFLAGS
    push    qword 0x1B      ; user CS
    push    r10             ; user RIP

    swapgs
    iretq
