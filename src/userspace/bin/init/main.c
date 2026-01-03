#include <stdio.h>
#include <syscall.h>
#include <stdint.h>


int main() {
    printf("Hello from userspace!\n");

    const char msg[] = "Hello from SYSCALL userland!\n";
    syscall(SYS_write, 1, (long)msg, sizeof(msg)-1);
    syscall(SYS_exit, 0, 0, 0);

    // long ret = syscall0(SYS_test);

    // printf("syscall returned %d\n", (int)ret);

    // syscall_exit(42);
    // while (1)
    // {
    //     // printf("Hello from userspace, while loop!\n");
    // }
    return 0;
}