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

[BITS 64]
global x64_syscall_entry
extern syscall_dispatch

section .text

x64_syscall_entry:
    swapgs

    ; Save syscall number and user RSP in caller-save regs
    mov     r9,  rax        ; r9 = syscall number
    mov     r8,  rsp        ; r8 = user RSP

    ; Build IRETQ frame on the user stack:
    ;   SS   = 0x23 (user data segment, RPL=3)
    ;   RSP  = original user RSP
    ;   RFLAGS = original user RFLAGS from SYSCALL (r11)
    ;   CS   = 0x1B (user code segment, RPL=3)
    ;   RIP  = original user RIP from SYSCALL (rcx)

    push    qword 0x23      ; SS
    push    r8              ; RSP
    push    r11             ; RFLAGS
    push    qword 0x1B      ; CS
    push    rcx             ; RIP

    ; Now rsp points to the IRETQ frame top (RIP).
    ; Prepare arguments for C:
    ; syscall_dispatch(uint64_t nr, uint64_t a0, uint64_t a1, uint64_t a2)
    ;
    ; SysV x86_64 calling convention:
    ;   rdi, rsi, rdx, rcx, r8, r9

    mov     rcx, rdx        ; arg3 (a2) = original rdx
    mov     rdx, rsi        ; arg2 (a1) = original rsi
    mov     rsi, rdi        ; arg1 (a0) = original rdi
    mov     rdi, r9         ; arg0 (nr) = saved syscall number

    call    syscall_dispatch    ; rax = return value to userspace

    ; At this point:
    ;   - rax holds syscall return value
    ;   - IRETQ frame is still on the user stack (pointed to by rsp)
    ; We don't need to restore any GPRs; they are caller-save for userspace.

    swapgs
    iretq                      ; pop RIP, CS, RFLAGS, RSP, SS and return to user



; [BITS 64]
; global x64_syscall_entry
; extern syscall_dispatch

; section .text
; x64_syscall_entry:
;     swapgs

;     ; Save user context in a fixed order
;     push rdi
;     push rsi
;     push rdx
;     push r10
;     push r8
;     push r9
;     push rcx        ; saved RIP
;     push r11        ; saved RFLAGS

;     ; rax still holds syscall number here

;     mov rsi, rsp    ; 2nd arg: syscall_regs_t *r
;     mov rdi, rax    ; 1st arg: uint64_t syscall_nr (SysV ABI: rdi, rsi)

;     call syscall_dispatch

;     ; Return value already in RAX

;     pop r11         ; restore RFLAGS
;     pop rcx         ; restore RIP
;     pop r9
;     pop r8
;     pop r10
;     pop rdx
;     pop rsi
;     pop rdi

;     swapgs
;     sysretq
