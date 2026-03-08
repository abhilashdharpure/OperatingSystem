global _start
extern main
extern _exit

_start:
    xor     ebp, ebp
    mov     rdi, [rsp]        ; argc
    lea     rsi, [rsp+8]      ; argv
    call    main
    mov     edi, eax
    call    _exit



; .global _start
; _start:
;     // System V AMD64 ABI: argc in %rdi, argv in %rsi, envp in %rdx if you want
;     // For now, just call main() with no args.

;     call main

;     // main() returns int in %eax
;     mov %eax, %edi        // exit code in %edi
;     mov $60, %eax         // SYS_exit on Linux; in your OS use your SYS_exit number
;     syscall

;     hlt                   // just in case
