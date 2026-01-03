; enter_user_mode.asm
; Jump from kernel to user mode (CPL=3) in x86_64
; Input: RDI = pointer to Process struct
;        Process.regs.rip = user entry
;        Process.regs.rsp = user stack top

[BITS 64]
global enter_user_mode

%define OFFSET_REGS_RIP 0
%define OFFSET_REGS_RSP 8

%define USER_CS 0x1B
%define USER_SS 0x23

section .text
enter_user_mode:
    cli
    mov rax, [rdi + OFFSET_REGS_RIP] ; user RIP
    mov rbx, [rdi + OFFSET_REGS_RSP] ; user RSP

    ; push iretq frame onto kernel stack
    push qword USER_SS
    push rbx
    pushfq
    pop rcx
    or rcx, 0x200
    push rcx
    push qword USER_CS
    push rax

    iretq

