[BITS 64]

global x64_syscall_entry

extern syscall_dispatch
extern g_syscall_rsp0

section .text

x64_syscall_entry:
    ; ------------------------------------------------------------
    ; CPU state on entry from user mode:
    ;
    ; RCX    = user RIP
    ; R11    = user RFLAGS
    ; RSP    = user RSP
    ; RAX    = syscall number
    ; RDI..R9 = syscall arguments
    ;
    ; SYSCALL does NOT save user RSP.
    ; ------------------------------------------------------------

    swapgs

    ; Save user RSP before changing RSP.
    mov     r12, rsp

    ; Switch to kernel syscall stack.
    mov     rsp, [rel g_syscall_rsp0]

    ; ------------------------------------------------------------
    ; Save registers that we will modify.
    ; ------------------------------------------------------------

    push    r12             ; user RSP

    push    rcx             ; user RIP
    push    r11             ; user RFLAGS

    push    rbx
    push    rbp
    push    r13
    push    r14
    push    r15

    ; ------------------------------------------------------------
    ; syscall_dispatch(nr, a0, a1, a2, a3, a4, a5)
    ;
    ; C ABI:
    ;   RDI = nr
    ;   RSI = a0
    ;   RDX = a1
    ;   RCX = a2
    ;   R8  = a3
    ;   R9  = a4
    ;
    ; a5 is passed on the stack if the C function really needs it.
    ;
    ; Linux-style syscall register mapping:
    ;   RAX = nr
    ;   RDI = a0
    ;   RSI = a1
    ;   RDX = a2
    ;   R10 = a3
    ;   R8  = a4
    ;   R9  = a5
    ; ------------------------------------------------------------

    mov     r15, r10        ; save a3
    mov     rbx, r8         ; save a4
    mov     rbp, r9         ; save a5

    mov     r9,  rbx        ; C arg4 = a4
    mov     r8,  r15        ; C arg3 = a3
    mov     rcx, rdx        ; C arg2 = a2
    mov     rdx, rsi        ; C arg1 = a1
    mov     rsi, rdi        ; C arg0 = a0
    mov     rdi, rax        ; C arg0 = syscall number

    ; C function has 7 arguments.
    ; The 7th argument (a5) is passed on the stack.
    sub     rsp, 8
    mov     [rsp], rbp

    call    syscall_dispatch

    add     rsp, 8

    ; ------------------------------------------------------------
    ; Return value:
    ;
    ; RAX = syscall return value
    ; ------------------------------------------------------------

    pop     r15
    pop     r14
    pop     r13
    pop     rbp
    pop     rbx

    pop     r11             ; user RFLAGS
    pop     rcx             ; user RIP
    pop     r12             ; user RSP

    ; ------------------------------------------------------------
    ; Build IRETQ frame:
    ;
    ;   SS
    ;   RSP
    ;   RFLAGS
    ;   CS
    ;   RIP
    ; ------------------------------------------------------------

    push    qword 0x23      ; USER_SS
    push    r12             ; user RSP
    push    r11             ; user RFLAGS
    push    qword 0x1B      ; USER_CS
    push    rcx             ; user RIP

    swapgs

    iretq