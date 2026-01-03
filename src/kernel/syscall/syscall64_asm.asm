; syscall64.asm
[BITS 64]
global x64_syscall_entry
extern syscall_dispatch

section .text

x64_syscall_entry:
    swapgs

    ; On entry:
    ;   rax = syscall number
    ;   rdi = arg0
    ;   rsi = arg1
    ;   rdx = arg2
    ;   rcx = user RIP
    ;   r11 = user RFLAGS
    ;   rsp = user RSP

    ; Save syscall number and user RSP
    mov     r9,  rax        ; r9 = syscall number
    mov     r8,  rsp        ; r8 = user RSP

    ; Build IRETQ frame on the user stack
    push    qword 0x23      ; SS (user data)
    push    r8              ; RSP (original user RSP)
    push    r11             ; RFLAGS
    push    qword 0x1B      ; CS (user code)
    push    rcx             ; RIP (original user RIP)

    ; Now set up C call:
    ;   syscall_dispatch(nr, a0, a1, a2)
    ; a0..a2 are still in rdi, rsi, rdx from user

    mov     rcx, rdx        ; arg3 (a2) = original rdx
    mov     rdx, rsi        ; arg2 (a1) = original rsi
    mov     rsi, rdi        ; arg1 (a0) = original rdi
    mov     rdi, r9         ; arg0 (nr) = saved syscall number

    call    syscall_dispatch
    ; rax now holds return value for userspace

    swapgs
    iretq
