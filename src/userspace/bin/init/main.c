#include <stdio.h>
#include <syscall.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/poll.h>

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

    // Make it read-only
    int r = mprotect(p, len, PROT_READ);
    printf("mprotect (RO) returned %d\n", r);

    // This write should now fault with a user-mode page fault
    // p[0] = 'Z'; // leave disabled for now
    int r2 = mprotect(p, len, PROT_READ | PROT_WRITE);
    printf("mprotect (RW) returned %d\n", r2);

    munmap(p, len);
}

void test_brk(void)
{
    void *cur = sbrk(0);


    printf("initial sbrk(0) = 0x%lx\n", (unsigned long)cur);

    void *p = sbrk(4096);
    printf("sbrk(+4096) returned 0x%lx\n", (unsigned long)p);

    char *c = (char *)p;
    for (int i = 0; i < 4; ++i)
        c[i] = "HEAP"[i];
    c[4] = '\n';
    syscall(SYS_write, 1, (long)c, 5);

    void *q = sbrk(-4096);
    printf("sbrk(-4096) returned 0x%lx\n", (unsigned long)q);
}

void test_poll(int fd)
{
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    pfd.revents = 0;

    int n = poll(&pfd, 1, 0); // timeout 0 = non-blocking

    printf("poll returned %d, revents=0x%x\n", n, pfd.revents);
}


int main()
{
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

    printf("Testing brk...\n");
    test_brk();

    printf("Testing poll...\n");
    fd = open("/folder/demo.txt", O_RDONLY, 0);
    printf("poll open returned fd=%d\n", fd);

    if (fd >= 0) {
        test_poll(fd);

        char buf[128];
        ssize_t n = read(fd, buf, sizeof(buf)-1);
        printf("poll read returned %d bytes\n", (int)n);

        close(fd);
    }


    printf("About to call SYS_exit via SYSCALL\n");
    syscall(SYS_exit, 0, 0, 0);
    printf("This should NEVER print\n");
    return 0;
}
