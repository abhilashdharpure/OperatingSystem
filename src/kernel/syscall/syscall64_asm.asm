; syscall64_asm.asm
[BITS 64]

global x64_syscall_entry

extern syscall_dispatch
extern g_syscall_rsp0

section .text

x64_syscall_entry:
    swapgs

    ; Save user RSP before switching to kernel stack
    mov     r12, rsp              ; r12 = user RSP

    ; Switch to kernel syscall stack
    mov     rsp, [rel g_syscall_rsp0]

    ; Save callee-saved regs
    push    rbx
    push    rbp
    push    r12
    push    r13
    push    r14
    push    r15

    ; Save volatile regs we want to restore later
    push    r8
    push    r9
    push    r10

    ; Save user RIP/RFLAGS exactly as given by SYSCALL
    push    rcx                   ; user RIP
    push    r11                   ; user RFLAGS

    ; --------- REORDER FOR C ABI (FIXED VERSION) ---------
    ; Linux syscall ABI on entry:
    ;   rax = nr
    ;   rdi = a0
    ;   rsi = a1
    ;   rdx = a2
    ;   r10 = a3
    ;   r8  = a4
    ;   r9  = a5

    ; Save args in temps
    mov     r12, rdi              ; r12 = a0
    mov     r13, rsi              ; r13 = a1
    mov     r14, rdx              ; r14 = a2
    mov     r15, r10              ; r15 = a3
    mov     rbx, r8               ; rbx = a4
    mov     rbp, r9               ; rbp = a5

    ; syscall_dispatch(nr, a0, a1, a2, a3, a4, a5)
    mov     rdi, rax              ; rdi = nr
    mov     rsi, r12              ; rsi = a0
    mov     rdx, r13              ; rdx = a1
    mov     rcx, r14              ; rcx = a2
    mov     r8,  r15              ; r8  = a3
    mov     r9,  rbx              ; r9  = a4
    push    rbp                   ; a5 on stack
    ; -----------------------------------------------------

    call    syscall_dispatch
    add     rsp, 8                ; pop a5

    ; Restore user RFLAGS and RIP from stack
    pop     r11                   ; user RFLAGS
    pop     rcx                   ; user RIP

    ; Restore saved volatile regs
    pop     r10
    pop     r9
    pop     r8

    ; Restore callee-saved regs
    pop     r15
    pop     r14
    pop     r13
    pop     r12                   ; r12 = user RSP
    pop     rbp
    pop     rbx

    ; Build iret frame with the *real* user context
    push    qword 0x23            ; user SS
    push    r12                   ; user RSP
    push    r11                   ; user RFLAGS
    push    qword 0x1B            ; user CS
    push    rcx                   ; user RIP

    swapgs
    iretq

;``````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````

; ; syscall64_asm.asm
; BITS 64

; global x64_syscall_entry

; extern syscall_dispatch
; extern g_syscall_rsp0
; extern debug_syscall_regs

; section .text

; ; On entry from SYSCALL:
; ;   rax = nr
; ;   rdi = a0
; ;   rsi = a1
; ;   rdx = a2
; ;   r10 = a3
; ;   r8  = a4
; ;   r9  = a5
; ;   rcx = user RIP
; ;   r11 = user RFLAGS
; ;   rsp = user RSP

; x64_syscall_entry:
;     swapgs

;     ; Save user context in registers
;     mov     r12, rsp      ; user RSP
;     mov     r13, rcx      ; user RIP
;     mov     r14, r11      ; user RFLAGS

;     ; Switch to kernel syscall stack
;     mov     rsp, [rel g_syscall_rsp0]

;     ; Push syscall state to kernel stack in a fixed layout:
;     ;   [rsp+72] = nr
;     ;   [rsp+64] = a0
;     ;   [rsp+56] = a1
;     ;   [rsp+48] = a2
;     ;   [rsp+40] = a3
;     ;   [rsp+32] = a4
;     ;   [rsp+24] = a5
;     ;   [rsp+16] = user RSP
;     ;   [rsp+8]  = user RIP
;     ;   [rsp]    = user RFLAGS

;     push    rax           ; nr
;     push    rdi           ; a0
;     push    rsi           ; a1
;     push    rdx           ; a2
;     push    r10           ; a3
;     push    r8            ; a4
;     push    r9            ; a5
;     push    r12           ; user RSP
;     push    r13           ; user RIP
;     push    r14           ; user RFLAGS

;     ; -------- debug_syscall_regs(nr,a0,a1,a2,a3,a4,a5,rsp_before_call) --------
;     ; Load args from the known stack layout

;     mov     rdi, [rsp+72] ; nr
;     mov     rsi, [rsp+64] ; a0
;     mov     rdx, [rsp+56] ; a1
;     mov     rcx, [rsp+48] ; a2
;     mov     r8,  [rsp+40] ; a3
;     mov     r9,  [rsp+32] ; a4

;     mov     r10, [rsp+24] ; a5
;     mov     r11, [rsp+16] ; rsp_before_call (user RSP)

;     sub     rsp, 16
;     mov     [rsp],     r10    ; a5
;     mov     [rsp+8],   r11    ; rsp_before_call

;     call    debug_syscall_regs

;     add     rsp, 16

;     ; -------- syscall_dispatch(nr,a0,a1,a2,a3,a4,a5) --------
;     ; Same layout, reload from stack

;     mov     rdi, [rsp+72] ; nr
;     mov     rsi, [rsp+64] ; a0
;     mov     rdx, [rsp+56] ; a1
;     mov     rcx, [rsp+48] ; a2
;     mov     r8,  [rsp+40] ; a3
;     mov     r9,  [rsp+32] ; a4

;     mov     r10, [rsp+24] ; a5

;     sub     rsp, 8
;     mov     [rsp], r10    ; a5 as 7th arg on stack

;     call    syscall_dispatch

;     add     rsp, 8

;     ; -------- Restore user context --------
;     ; Stack still:
;     ;   [rsp+72] = nr
;     ;   [rsp+64] = a0
;     ;   [rsp+56] = a1
;     ;   [rsp+48] = a2
;     ;   [rsp+40] = a3
;     ;   [rsp+32] = a4
;     ;   [rsp+24] = a5
;     ;   [rsp+16] = user RSP
;     ;   [rsp+8]  = user RIP
;     ;   [rsp]    = user RFLAGS

;     pop     r14           ; user RFLAGS
;     pop     r13           ; user RIP
;     pop     r12           ; user RSP

;     ; Drop nr + a0..a5
;     add     rsp, 7*8

;     ; Build iret frame
;     push    qword 0x23    ; user SS
;     push    r12           ; user RSP
;     push    r14           ; user RFLAGS
;     push    qword 0x1B    ; user CS
;     push    r13           ; user RIP

;     swapgs
;     iretq


;-------------------------------------------------------------------------------------------------------------------------------------------------------

; ; syscall64_asm.asm
; ; syscall64_asm.asm
; [BITS 64]

; global x64_syscall_entry

; extern syscall_dispatch
; extern g_syscall_rsp0

; ; C prototype (for reference):
; ; void debug_syscall_regs(uint64_t nr,
; ;                         uint64_t a0,
; ;                         uint64_t a1,
; ;                         uint64_t a2,
; ;                         uint64_t a3,
; ;                         uint64_t a4,
; ;                         uint64_t a5,
; ;                         uint64_t rsp_before_call);
; extern debug_syscall_regs

; section .text

; ; On entry from SYSCALL (Linux x86_64):
; ;   rax = nr
; ;   rdi = a0
; ;   rsi = a1
; ;   rdx = a2
; ;   r10 = a3
; ;   r8  = a4
; ;   r9  = a5
; ;   rcx = user RIP
; ;   r11 = user RFLAGS
; ;   rsp = user RSP

; x64_syscall_entry:
;     swapgs

;     ; Save user RSP before switching to kernel stack
;     mov     r12, rsp              ; r12 = user RSP

;     ; Switch to kernel syscall stack
;     mov     rsp, [rel g_syscall_rsp0]

;     ; Save callee-saved regs we’ll clobber
;     push    rbx
;     push    rbp
;     push    r12
;     push    r13
;     push    r14
;     push    r15

;     ; Save user RIP/RFLAGS exactly as given by SYSCALL
;     push    rcx                   ; [bottom] user RIP
;     push    r11                   ; [above]  user RFLAGS

;     ; --------- SNAPSHOT SYSCALL ARGS INTO TEMPS ---------
;     mov     r13, rax              ; r13 = nr
;     mov     r14, rdi              ; r14 = a0
;     mov     r15, rsi              ; r15 = a1
;     mov     rbx, rdx              ; rbx = a2
;     mov     rbp, r10              ; rbp = a3
;     mov     r10, r8               ; r10 = a4
;     mov     r11, r9               ; r11 = a5

;     ; --------- debug_syscall_regs(nr,a0,a1,a2,a3,a4,a5,rsp_before_call) ---------
;     mov     rdi, r13              ; nr
;     mov     rsi, r14              ; a0
;     mov     rdx, r15              ; a1
;     mov     rcx, rbx              ; a2
;     mov     r8,  rbp              ; a3
;     mov     r9,  r10              ; a4

;     sub     rsp, 16
;     mov     [rsp],    r11         ; 7th arg: a5
;     mov     [rsp+8],  r12         ; 8th arg: user RSP before syscall

;     call    debug_syscall_regs

;     add     rsp, 16               ; drop a5,rsp_before_call arg slots

;     ; --------- syscall_dispatch(nr,a0,a1,a2,a3,a4,a5) ---------
;     mov     rdi, r13              ; rdi = nr
;     mov     rsi, r14              ; rsi = a0
;     mov     rdx, r15              ; rdx = a1
;     mov     rcx, rbx              ; rcx = a2
;     mov     r8,  rbp              ; r8  = a3
;     mov     r9,  r10              ; r9  = a4

;     push    r11                   ; a5 on stack (7th arg)
;     call    syscall_dispatch
;     add     rsp, 8                ; pop a5 arg slot

;     ; --------- RESTORE USER CONTEXT ---------
;     pop     r11                   ; user RFLAGS
;     pop     rcx                   ; user RIP

;     ; Restore callee-saved regs
;     pop     r15
;     pop     r14
;     pop     r13
;     pop     r12                   ; user RSP
;     pop     rbp
;     pop     rbx

;     ; Build iret frame with the *real* user context
;     push    qword 0x23            ; user SS
;     push    r12                   ; user RSP
;     push    r11                   ; user RFLAGS
;     push    qword 0x1B            ; user CS
;     push    rcx                   ; user RIP

;     swapgs
;     iretq

;-------------------------------------------------------------------------------------------------------------------------------------------------------

; ; syscall64_asm.asm
; ; syscall64_asm.asm
; [BITS 64]

; global x64_syscall_entry

; extern syscall_dispatch
; extern g_syscall_rsp0

; ; C prototype (for reference):
; ; void debug_syscall_regs(uint64_t nr,
; ;                         uint64_t a0,
; ;                         uint64_t a1,
; ;                         uint64_t a2,
; ;                         uint64_t a3,
; ;                         uint64_t a4,
; ;                         uint64_t a5,
; ;                         uint64_t rsp_before_call);
; extern debug_syscall_regs

; section .text

; ; On entry from SYSCALL (Linux x86_64):
; ;   rax = nr
; ;   rdi = a0
; ;   rsi = a1
; ;   rdx = a2
; ;   r10 = a3
; ;   r8  = a4
; ;   r9  = a5
; ;   rcx = user RIP
; ;   r11 = user RFLAGS
; ;   rsp = user RSP

; x64_syscall_entry:
;     swapgs

;     ; Save user RSP before switching to kernel stack
;     mov     r12, rsp              ; r12 = user RSP

;     ; Switch to kernel syscall stack
;     mov     rsp, [rel g_syscall_rsp0]

;     ; Save callee-saved regs we’ll clobber
;     push    rbx
;     push    rbp
;     push    r12
;     push    r13
;     push    r14
;     push    r15

;     ; Save user RIP/RFLAGS exactly as given by SYSCALL
;     push    rcx                   ; [bottom] user RIP
;     push    r11                   ; [above]  user RFLAGS

;     ; --------- SNAPSHOT SYSCALL ARGS INTO TEMPS ---------
;     mov     r13, rax              ; r13 = nr
;     mov     r14, rdi              ; r14 = a0
;     mov     r15, rsi              ; r15 = a1
;     mov     rbx, rdx              ; rbx = a2
;     mov     rbp, r10              ; rbp = a3
;     mov     r10, r8               ; r10 = a4
;     mov     r11, r9               ; r11 = a5

;     ; save a4/a5 in stack slots (callee-saved across C calls)
;     sub     rsp, 16
;     mov     [rsp],    r10         ; save a4
;     mov     [rsp+8],  r11         ; save a5

;     ; --------- debug_syscall_regs(nr,a0,a1,a2,a3,a4,a5,rsp_before_call) ---------
;     mov     rdi, r13              ; nr
;     mov     rsi, r14              ; a0
;     mov     rdx, r15              ; a1
;     mov     rcx, rbx              ; a2
;     mov     r8,  rbp              ; a3
;     mov     r9,  r10              ; a4 (just for the debug call)

;     sub     rsp, 16
;     mov     [rsp],    r11         ; 7th arg: a5
;     mov     [rsp+8],  r12         ; 8th arg: user RSP before syscall

;     call    debug_syscall_regs

;     add     rsp, 16               ; drop a5,rsp_before_call

;     ; restore a4/a5 after debug call (they may have been clobbered)
;     mov     r10, [rsp]            ; a4
;     mov     r11, [rsp+8]          ; a5
;     add     rsp, 16               ; drop saved a4/a5

;     ; --------- syscall_dispatch(nr,a0,a1,a2,a3,a4,a5) ---------
;     mov     rdi, r13              ; nr
;     mov     rsi, r14              ; a0
;     mov     rdx, r15              ; a1
;     mov     rcx, rbx              ; a2
;     mov     r8,  rbp              ; a3
;     mov     r9,  r10              ; a4
;     push    r11                   ; a5 on stack
;     call    syscall_dispatch
;     add     rsp, 8


;     ; --------- RESTORE USER CONTEXT ---------
;     pop     r11                   ; user RFLAGS
;     pop     rcx                   ; user RIP

;     ; Restore callee-saved regs
;     pop     r15
;     pop     r14
;     pop     r13
;     pop     r12                   ; user RSP
;     pop     rbp
;     pop     rbx

;     ; Build iret frame with the *real* user context
;     push    qword 0x23            ; user SS
;     push    r12                   ; user RSP
;     push    r11                   ; user RFLAGS
;     push    qword 0x1B            ; user CS
;     push    rcx                   ; user RIP

;     swapgs
;     iretq


; syscall64.asm
; 64-bit syscall entry stub
;
; Entry from userspace (long mode):
;   rax = syscall number
;   rdi = arg0
;   rsi = arg1
;   rdx = arg2
;   rcx = user RIP (next instruction after syscall)
;   r11 = user RFLAGS
;   rsp = user RSP
;
; We will:
;   - build an IRETQ frame on the user stack
;   - call C: uint64_t syscall_dispatch(uint64_t nr, uint64_t a0, uint64_t a1, uint64_t a2);
;   - return with iretq











; ; syscall64_asm.asm
; [BITS 64]

; global x64_syscall_entry

; extern syscall_dispatch
; extern g_syscall_rsp0

; section .text

; x64_syscall_entry:
;     swapgs

;     ; Save user RSP before switching to kernel stack
;     mov     r12, rsp              ; r12 = user RSP

;     ; Switch to kernel syscall stack
;     mov     rsp, [rel g_syscall_rsp0]

;     ; Save callee-saved regs
;     push    rbx
;     push    rbp
;     push    r12
;     push    r13
;     push    r14
;     push    r15

;     ; Save volatile regs we want to restore later
;     push    r8
;     push    r9
;     push    r10

;     ; Save user RIP/RFLAGS exactly as given by SYSCALL
;     push    rcx                   ; user RIP
;     push    r11                   ; user RFLAGS

;     ; --------- REORDER FOR C ABI (FIXED VERSION) ---------
;     ; Linux syscall ABI on entry:
;     ;   rax = nr
;     ;   rdi = a0
;     ;   rsi = a1
;     ;   rdx = a2
;     ;   r10 = a3
;     ;   r8  = a4
;     ;   r9  = a5

;     ; Save args in temps
; ; on entry: rax=nr, rdi=a0, rsi=a1, rdx=a2, r10=a3, r8=a4, r9=a5

;     mov r12, rdi   ; a0
;     mov r13, rsi   ; a1
;     mov r14, rdx   ; a2
;     mov r15, r10   ; a3
;     mov rbx, r8    ; a4
;     mov rbp, r9    ; a5

;     mov rdi, rax   ; nr
;     mov rsi, r12   ; a0
;     mov rdx, r13   ; a1
;     mov rcx, r14   ; a2
;     mov r8,  r15   ; a3
;     mov r9,  rbx   ; a4
;     push rbp       ; a5 on stack
;     call syscall_dispatch

;     add     rsp, 8                ; pop a5

;     ; Restore user RFLAGS and RIP from stack
;     pop     r11                   ; user RFLAGS
;     pop     rcx                   ; user RIP

;     ; Restore saved volatile regs
;     pop     r10
;     pop     r9
;     pop     r8

;     ; Restore callee-saved regs
;     pop     r15
;     pop     r14
;     pop     r13
;     pop     r12                   ; r12 = user RSP
;     pop     rbp
;     pop     rbx

;     ; Build iret frame with the *real* user context
;     push    qword 0x23            ; user SS
;     push    r12                   ; user RSP
;     push    r11                   ; user RFLAGS
;     push    qword 0x1B            ; user CS
;     push    rcx                   ; user RIP

;     swapgs
;     iretq
