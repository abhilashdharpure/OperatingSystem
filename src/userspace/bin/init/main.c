#include <stdio.h>
#include <syscall.h>
#include <stdint.h>

int main() {
    printf("Hello from userspace!\n");
    while (1)
    {
        // printf("Hello from userspace, while loop!\n");
    }
    return 0;
}