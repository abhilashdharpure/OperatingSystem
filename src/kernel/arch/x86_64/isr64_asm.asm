; isr64_asm.asm
[BITS 64]
global x64_ISR_CommonStub
extern x64_ISR_CommonHandler

section .text

x64_ISR_CommonStub:
    ; For now: treat UD2 as “no error code”
    push qword 0          ; dummy error code
    push qword 0xFF       ; dummy vector

    ; Optionally save some caller-saved regs if your C handler expects them intact:
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11

    mov rdi, rsp          ; arg to C: pointer to frame
    call x64_ISR_CommonHandler

    ; restore caller-saved registers
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rax

    add rsp, 16           ; pop vector + error code
    iretq
