.global switch_to_user_mode
.type switch_to_user_mode, @function

switch_to_user_mode:
    cli                     # disable interrupts
    mov ax, 0x23            # user data segment selector (ring 3)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, esp            # save current kernel stack
    push 0x23               # user data segment selector (SS)
    push eax                # user stack pointer
    pushf                   # EFLAGS
    push 0x1B               # user code segment selector (CS)
    push user_entry         # user instruction pointer
    iret                    # switches to ring 3
