.section .text.crt0,"ax"
.globl _start
.extern main
.extern syscall_exit

_start:
    call main              # int main(void)

    # SysV AMD64: return value is in %rax.
    # Pass it as first arg to syscall_exit(int status) in %rdi.
    mov    %rax, %rdi
    call   syscall_exit

1:  jmp 1b                 # if syscall_exit returns, spin
