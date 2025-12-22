; isr64_asm.asm
[BITS 64]
global x64_ISR_CommonStub
extern x64_ISR_CommonHandler

section .text

; This is a simple version that assumes no error code, just to get started.
; We’ll extend it later if you want full exception semantics.

x64_ISR_CommonStub:
    ; CPU has pushed: RIP, CS, RFLAGS, RSP, SS (for CPL change)
    ; We add a dummy "vector" field for now (0xFF).
    push qword 0xFF
    mov rdi, rsp                 ; first arg: pointer to frame (InterruptFrame64)
    call x64_ISR_CommonHandler

    add rsp, 8                   ; pop vector
    iretq
