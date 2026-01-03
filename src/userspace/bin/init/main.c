#include <stdio.h>
#include <syscall.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    printf("Hello from userspace!\n");

    const char msg[] = "Hello from SYSCALL userland!\n";
    syscall(SYS_write, 1, (long)msg, sizeof(msg)-1);

    printf("About to test open/read/close via SYSCALL\n");

    int fd = open("/folder/demo.txt", O_RDONLY, 0);
    printf("open returned fd=%d\n", fd);

    if (fd >= 0) {
        char buf[128];
        ssize_t n = read(fd, buf, sizeof(buf)-1);

        if (n > 0) {
            buf[n] = '\0';
            printf("read returned %d bytes\n", (int)n);
            printf("buffer: '%s'\n", buf);
        } else {
            printf("read returned error: %d\n", (int)n);
        }

        close(fd);
    }

    printf("About to call SYS_exit via SYSCALL\n");
    syscall(SYS_exit, 0, 0, 0);
    printf("This should NEVER print\n");
    return 0;
}
