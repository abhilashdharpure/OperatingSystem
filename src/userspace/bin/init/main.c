#include <stdio.h>
#include <syscall.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>

// Prot flags (mirror Linux for future compatibility)
#define PROT_READ   0x1
#define PROT_WRITE  0x2

// Map flags
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20

void test_mmap(void) {
    size_t len = 4096;
    char *p = mmap(NULL, len, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS,
                   -1, 0);

    if (p == (void *)-1) {
        printf("mmap failed\n");
        return;
    }

    for (int i = 0; i < 4; ++i)
        p[i] = "ABCD"[i];

    p[4] = '\0';
    printf("mmap buffer: '%s'\n", p);
}


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

    printf("Testing mmap...\n");
    test_mmap();

    printf("About to call SYS_exit via SYSCALL\n");
    syscall(SYS_exit, 0, 0, 0);
    printf("This should NEVER print\n");
    return 0;
}
