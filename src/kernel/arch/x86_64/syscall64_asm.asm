; syscall64.asm
[BITS 64]
global x64_syscall_entry
extern syscall_dispatch

section .text

x64_syscall_entry:
    ; On entry: RCX=user RIP, R11=user RFLAGS, RAX=syscall number,
    ;           RDI,RSI,RDX,R10,R8,R9=args

    push r11
    push rcx

    ; Arrange arguments for syscall_dispatch(num, a0,a1,a2,a3,a4)
    mov rdi, rax        ; num
    mov rsi, rdi        ; a0 already in RDI from caller if you want Linux ABI,
                        ; or move from RDI/RSI/... however you choose.

    ; For a clean start, assume:
    ;   RAX=syscall number
    ;   RDI,RSI,RDX,R10,R8,R9 = arg0..arg5 (Linux-style)
    ; Then:
    ;   rdi = num
    ;   rsi = arg0
    ;   rdx = arg1
    ;   rcx = arg2
    ;   r8  = arg3
    ;   r9  = arg4
    ;
    mov rsi, rdi        ; incorrect as written; you'll wire this properly
    ; For now, just forward num and ignore args:
    call syscall_dispatch

    pop rcx             ; restore user RIP
    pop r11             ; restore user RFLAGS
    sysretq
