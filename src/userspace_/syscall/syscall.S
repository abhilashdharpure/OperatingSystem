.global syscall

syscall:
    mov 4(%esp), %eax   // syscall number
    mov 8(%esp), %ebx   // arg1
    mov 12(%esp), %ecx  // arg2
    mov 16(%esp), %edx  // arg3
    int $0x80
    ret
