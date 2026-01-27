; isr64_asm.asm
; [BITS 64]
; global x64_isr_common
; extern x64_ISR_Handler

; section .text

; %macro ISR64_NOERR 1
; global x64_isr_%1
; x64_isr_%1:
;     push qword 0          ; error code (lower address)
;     push qword %1         ; vector (higher address)
;     jmp x64_isr_common
; %endmacro

; %macro ISR64_ERR 1
; global x64_isr_%1
; x64_isr_%1:
;     ; CPU already pushed error code
;     push qword %1         ; vector (above error)
;     jmp x64_isr_common
; %endmacro

; %include "src/kernel/arch/x86_64/isrs64_gen.inc"

; x64_isr_common:
;     ; push regs, last push = rax (lowest address in struct)
;     push r15
;     push r14
;     push r13
;     push r12
;     push r11
;     push r10
;     push r9
;     push r8
;     push rbp
;     push rdi
;     push rsi
;     push rbx
;     push rdx
;     push rcx
;     push rax

;     mov rdi, rsp          ; &ISRFrame64
;     call x64_ISR_Handler

;     pop rax
;     pop rcx
;     pop rdx
;     pop rbx
;     pop rsi
;     pop rdi
;     pop rbp
;     pop r8
;     pop r9
;     pop r10
;     pop r11
;     pop r12
;     pop r13
;     pop r14
;     pop r15

;     add rsp, 16           ; pop vector + error
;     iretq

; isr64_asm.asm
[BITS 64]
global x64_isr_common
extern x64_ISR_Handler

section .text

%macro ISR64_NOERR 1
global x64_isr_%1
x64_isr_%1:
    push qword 0          ; error code (CPU didn't push one)
    push qword %1         ; vector
    jmp x64_isr_common
%endmacro

%macro ISR64_ERR 1
global x64_isr_%1
x64_isr_%1:
    ; CPU already pushed error code
    push qword %1         ; vector (above error)
    jmp x64_isr_common
%endmacro

%include "src/kernel/arch/x86_64/isrs64_gen.inc"

x64_isr_common:
    ; push regs, last push = rax (highest address)
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rdi
    push rsi
    push rbx
    push rdx
    push rcx
    push rax

    ; rsp now points at rax; CPU frame is 17 qwords below
    lea     rdi, [rsp - 17*8]   ; &ISRFrame64 (cpu at lowest address)
    call    x64_ISR_Handler

    ; restore regs
    pop rax
    pop rcx
    pop rdx
    pop rbx
    pop rsi
    pop rdi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    add rsp, 16                 ; pop vector + error
    iretq
