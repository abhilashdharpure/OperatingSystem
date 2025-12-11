.global syscall_entry

syscall_entry:
    pusha

    push %ds
    push %es
    push %fs
    push %gs

    mov $0x10, %ax        // kernel data segment
    mov %ax, %ds
    mov %ax, %es

    push %esp             // pointer to registers
    call syscall_dispatch
    add $4, %esp

    pop %gs
    pop %fs
    pop %es
    pop %ds

    popa
    iret
