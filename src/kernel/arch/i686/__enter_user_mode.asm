; enter_user_mode.asm
; NASM syntax, 32-bit protected mode
; call convention (cdecl):
;   [esp+4] = user_eip
;   [esp+8] = user_esp
BITS 32
global enter_user_mode
section .text

enter_user_mode:
    ; read arguments from caller stack
    mov    eax, [esp + 4]    ; eax = user_eip
    mov    ebx, [esp + 8]    ; ebx = user_esp

    cli                      ; disable interrupts while we prepare

    ; load user data selector into data segments (0x23 typical)
    mov    ax, 0x23
    mov    ds, ax
    mov    es, ax
    mov    fs, ax
    mov    gs, ax

    ; push user iret frame:
    ;   SS
    ;   ESP
    ;   EFLAGS (with IF=1)
    ;   CS
    ;   EIP

    push   dword 0x23        ; user SS (selector with RPL=3)
    push   ebx               ; user ESP

    pushfd                  ; push EFLAGS
    pop    ecx              ; ecx = original eflags
    or     ecx, 0x200       ; set IF bit
    push   ecx              ; push modified eflags

    push   dword 0x1B       ; user CS (selector with RPL=3)
    push   eax              ; user EIP (entry)

    iretd                   ; far return -> transitions to ring 3, sets CS:EIP, EFLAGS, SS:ESP

    hlt                     ; should never reach here
