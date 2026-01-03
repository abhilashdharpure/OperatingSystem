; syscall64.asm
[BITS 64]
global x64_syscall_entry
extern syscall_dispatch

section .text
x64_syscall_entry:
    swapgs

    ; Save user context
    push rdi
    push rsi
    push rdx
    push r10
    push r8
    push r9
    push r11        ; saved RFLAGS
    push rcx        ; saved RIP

    mov rdi, rsp    ; syscall_regs_t *
    call syscall_dispatch

    ; Return value already in RAX

    pop rcx
    pop r11
    pop r9
    pop r8
    pop r10
    pop rdx
    pop rsi
    pop rdi

    swapgs
    sysretq
