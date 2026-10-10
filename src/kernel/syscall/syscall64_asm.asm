[BITS 64]
global x64_syscall_entry
global ret_from_fork
global switch_context
extern syscall_dispatch
extern g_syscall_rsp0

section .bss
align 8
g_user_rsp_tmp: resq 1

section .text
x64_syscall_entry:
    swapgs
    mov     [rel g_user_rsp_tmp], rsp
    mov     rsp, [rel g_syscall_rsp0]          ; per-process, 16-byte aligned

    push    qword 0x23                         ; SS
    push    qword [rel g_user_rsp_tmp]         ; RSP
    push    r11                                ; RFLAGS
    push    qword 0x1B                         ; CS
    push    rcx                                ; RIP
    push    rbx
    push    rbp
    push    r12
    push    r13
    push    r14
    push    r15
    push    rdi
    push    rsi
    push    rdx
    push    r10
    push    r8
    push    r9
    mov     rbx, rsp                           ; rbx = &frame (callee-saved)

    ; 17 pushes = 136 B (8 mod 16). pad + 2 stack args -> aligned at call
    sub     rsp, 8
    push    rbx                                ; arg8: frame*
    push    r9                                 ; arg7: a5
    mov     r9,  r8                            ; a4
    mov     r8,  r10                           ; a3
    mov     rcx, rdx                           ; a2
    mov     rdx, rsi                           ; a1
    mov     rsi, rdi                           ; a0
    mov     rdi, rax                           ; nr
    call    syscall_dispatch
    add     rsp, 24
    jmp     syscall_exit

; First instruction executed by a new child. rsp -> struct syscall_frame
ret_from_fork:
    xor     eax, eax                           ; fork() returns 0 in the child
syscall_exit:
    pop     r9
    pop     r8
    pop     r10
    pop     rdx
    pop     rsi
    pop     rdi
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbp
    pop     rbx
    swapgs
    iretq

; void switch_context(uint64_t *save_rsp /*rdi*/, uint64_t new_rsp /*rsi*/)
switch_context:
    push    rbx
    push    rbp
    push    r12
    push    r13
    push    r14
    push    r15
    mov     [rdi], rsp
    mov     rsp, rsi
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbp
    pop     rbx
    ret