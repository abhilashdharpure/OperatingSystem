; paging_asm.asm
bits 64
global enter_user_mode

; void enter_user_mode(Process *p);
; p in RDI

%define OFFSET_REGS_RIP  0    ; adjust if Process has fields before regs
%define OFFSET_REGS_RSP  8

%define USER_CS  0x1B   ; user code
%define USER_DS  0x23   ; user data


section .text
enter_user_mode:
    cli

    ; Load full 64-bit RIP/RSP
    mov     rax, [rdi + OFFSET_REGS_RIP]   ; user RIP (64-bit)
    mov     rbx, [rdi + OFFSET_REGS_RSP]   ; user RSP (64-bit)

    ; Build 64-bit iret frame: SS, RSP, RFLAGS, CS, RIP

    push    qword USER_DS      ; SS (64-bit push)
    push    rbx                ; RSP

    pushfq
    pop     rcx
    or      rcx, 0x200         ; IF=1
    push    rcx                ; RFLAGS

    push    qword USER_CS      ; CS
    push    rax                ; RIP

    iretq
