#include <stdio.h>
#include <syscall.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

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

void print_ns(int ns)
{
    if (ns < 0) ns += 1000000000;

    int width = 9;
    int tmp = ns;

    while (tmp > 0) {
        tmp /= 10;
        width--;
    }

    while (width-- > 0)
        printf("0");

    printf("%d", ns);
}



int main()
{
    klog("Hello from userspace!\n");
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


    printf("Testing stat...\n");

    struct stat st;
    if (stat("/folder/demo.txt", &st) == 0)
    {
        printf("stat: size=%d inode=%d\n",
       (int)st.st_size,
       (int)st.st_ino);
    } 
    else
    {
        printf("stat failed\n");
    }

    printf("Testing lseek...\n");
    int fd2 = open("/folder/demo.txt", O_RDONLY, 0);
    printf("lseek open returned fd=%d\n", fd2);

    if (fd2 >= 0) {
        char buf[8] = {0};

        // Seek to offset 2
        off_t off = lseek(fd2, 2, 0); // SEEK_SET = 0
        printf("lseek returned %ld\n", off);

        ssize_t n = read(fd2, buf, 4);
        printf("lseek read returned %d bytes, buf='%s'\n", (int)n, buf);

        close(fd2);
    }

    printf("Testing getdents on '/'\n");

    struct dirent ents[16];
    int n = getdents("/", ents, 16);

    printf("getdents returned %d\n", n);
    for (int i = 0; i < n; ++i) {
        printf("  [%d] ino=%d type=%d name=%s\n",
            i,
            (int)ents[i].d_ino,
            (int)ents[i].d_type,
            ents[i].d_name);
    }

    printf("Testing dup...\n");
    fd = open("/folder/demo.txt", O_RDONLY, 0);
    printf("dup open returned fd=%d\n", fd);

    fd2 = dup(fd);
    printf("dup returned fd2=%d\n", fd2);

    char buf1[8] = {0};
    char buf2[8] = {0};

    read(fd, buf1, 2);   // read "De"
    read(fd2, buf2, 2);  // should continue from same position -> "mo"

    printf("fd read='%s', fd2 read='%s'\n", buf1, buf2);

    close(fd);
    close(fd2);

    printf("Testing dup2...\n");
    int fd3 = open("/folder/demo.txt", O_RDONLY, 0);
    int fd4 = 10;
    int r = dup2(fd3, fd4);
    printf("dup2 returned %d, new fd=%d\n", r, fd4);
    close(fd3);
    close(fd4);



    printf("Testing fcntl...\n");
    fd = open("/folder/demo.txt", O_RDONLY, 0);
    printf("fcntl open returned fd=%d\n", fd);

    int flags = fcntl(fd, F_GETFL, 0);
    printf("F_GETFL returned 0x%x\n", flags);

    r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    printf("F_SETFL returned %d\n", r);

    int flags2 = fcntl(fd, F_GETFL, 0);
    printf("F_GETFL (after) returned 0x%x\n", flags2);

    close(fd);


    printf("Testing pipe...\n");

    int fds[2];
    if (pipe(fds) == 0) {
        printf("pipe created: r=%d w=%d\n", fds[0], fds[1]);

        const char *msg = "Hello via pipe!\n";
        ssize_t wn = write(fds[1], msg, 15);
        printf("pipe write returned %d\n", (int)wn);

        char buf[32] = {0};
        ssize_t rn = read(fds[0], buf, sizeof(buf)-1);
        printf("pipe read returned %d, buf='%s'\n", (int)rn, buf);

        close(fds[0]);
        close(fds[1]);
    } else {
        printf("pipe failed\n");
    }

    klog("Testing clock_gettime...\n");

    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
    {
        printf("CLOCK_MONOTONIC: %d.%d sec\n", (int)ts.tv_sec, (int)ts.tv_nsec);

    } 
    else {
        printf("clock_gettime failed\n");
        klog("clock_gettime failed\n");
    }

    if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
    {
        printf("CLOCK_REALTIME: %d.%d sec\n", (int)ts.tv_sec, (int)ts.tv_nsec);
    }


    printf("Testing nanosleep...\n");

    struct timespec req;
    req.tv_sec  = 0;
    req.tv_nsec = 500000000L; // 0.5 sec

    struct timespec before, after;
    clock_gettime(CLOCK_MONOTONIC, &before);

    nanosleep(&req, NULL);

    clock_gettime(CLOCK_MONOTONIC, &after);
    printf("Slept: %d.", (int)(after.tv_sec - before.tv_sec));
    print_ns((int)(after.tv_nsec - before.tv_nsec));
    printf(" sec (approx)\n");


    printf("About to call SYS_exit via SYSCALL\n");
    syscall(SYS_exit, 0, 0, 0);
    printf("This should NEVER print\n");
    return 0;
}
