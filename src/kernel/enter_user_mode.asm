; enter_user_mode.asm
[BITS 64]
global enter_user_mode

%define OFFSET_REGS_RIP 0
%define OFFSET_REGS_RSP 8

%define USER_CS 0x1B
%define USER_SS 0x23

enter_user_mode:
    cli

    mov     rax, [rdi + OFFSET_REGS_RIP]   ; user RIP
    mov     rbx, [rdi + OFFSET_REGS_RSP]   ; user RSP

    cld

    pushfq
    pop     rcx
    and     rcx, ~(1 << 10)        ; clear DF
    or      rcx,  (1 << 9)         ; set IF

    push    qword USER_SS
    push    rbx                    ; user RSP
    push    rcx                    ; user RFLAGS
    push    qword USER_CS
    push    rax                    ; user RIP

    iretq
