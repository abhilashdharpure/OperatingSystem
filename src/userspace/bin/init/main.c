// #include <stdio.h>
#include <syscall.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <sys/socket.h>
#include <string.h>

#include <sys/un.h>
#include "uprintf.h"
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/execve.h>

uint64_t get_time_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (ts.tv_nsec / 1000000ULL);
}


static void klog_hex(const char *label, long v)
{
    char buf[64];
    char *p = buf;
    *p++ = '[';
    while (*label) *p++ = *label++;
    *p++ = ' ';
    *p++ = '0'; *p++ = 'x';

    const char *hex = "0123456789abcdef";
    for (int i = (sizeof(long)*2)-1; i >= 0; --i) {
        *p++ = hex[(v >> (i*4)) & 0xf];
    }
    *p++ = ']';
    *p++ = '\n';
    *p = 0;
    klog(buf);
}

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

// void *my_sbrk(intptr_t increment)
// {
//     long cur = syscall6(SYS_brk, 0, 0, 0, 0);
//     // klog("my_sbrk: cur=0x%lx\n", cur);
//     printf("my_sbrk: cur=0x%lx\n", cur);

//     if (cur == 0)
//         return (void *)-1;

//     long new = cur + increment;
//     long ret = syscall6(SYS_brk, new, 0, 0, 0);
//     // klog("my_sbrk: new=0x%lx ret=0x%lx\n", new, ret);
//     printf("my_sbrk: new=0x%lx ret=0x%lx\n", new, ret);

//     if (ret != new)
//         return (void *)-1;

//     return (void *)cur;
// }

void *sbrk(intptr_t increment)
{
    long cur = syscall6(SYS_brk, 0, 0, 0, 0, 0, 0);
    if (cur == 0)
        return (void *)-1;

    long new = cur + increment;
    long ret = syscall6(SYS_brk, new, 0, 0, 0, 0, 0);

    if (ret != new)
        return (void *)-1;

    return (void *)cur;
}


void test_brk(void)
{
    printf("Calling brk test\n");

    void *cur = sbrk(0);


    printf("initial sbrk(0) = 0x%lx\n", (unsigned long)cur);

    void *p = sbrk(4096);
    printf("sbrk(+4096) returned 0x%lx\n", (unsigned long)p);

    // char *c = (char *)p;
    // for (int i = 0; i < 4; ++i)
    //     c[i] = "HEAP"[i];
    // c[4] = '\n';
    // syscall6(SYS_write, 1, (long)c, 5, 0);

    // void *q = sbrk(-4096);
    // printf("sbrk(-4096) returned 0x%lx\n", (unsigned long)q);
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

static void test_socketpair_poll(void)
{
    printf("Testing socketpair + poll...\n");

    int sv[2];
    printf("sv at %p\n", (void*)sv);
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) != 0) {
        printf("socketpair failed\n");
        return;
    }

    struct pollfd fds[2];
    fds[0].fd = sv[0];
    fds[0].events = POLLIN | POLLOUT;
    fds[1].fd = sv[1];
    fds[1].events = POLLIN | POLLOUT;

    printf("Initial poll (no data)...\n");
    int ret = poll(fds, 2, 0);
    printf("poll returned %d\n", ret);
    printf("fd0 revents=%x fd1 revents=%x\n", fds[0].revents, fds[1].revents);

    printf("Writing from sv[0] to sv[1]...\n");
    write(sv[0], "POLL", 4);

    memset(fds, 0, sizeof(fds));
    fds[0].fd = sv[0];
    fds[0].events = POLLIN | POLLOUT;
    fds[1].fd = sv[1];
    fds[1].events = POLLIN | POLLOUT;

    ret = poll(fds, 2, 0);
    printf("After write, poll returned %d\n", ret);
    printf("fd0 revents=%x fd1 revents=%x\n", fds[0].revents, fds[1].revents);

    char buf[8] = {0};
    int n = read(sv[1], buf, sizeof(buf));
    printf("read on sv[1] -> n=%d buf='%s'\n", n, buf);

    close(sv[0]);
    close(sv[1]);
}

static void test_socketpair_nonblock(void)
{
    printf("Testing socketpair + O_NONBLOCK...\n");

    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) != 0) {
        printf("socketpair failed\n");
        return;
    }

    int flags = fcntl(sv[1], F_GETFL, 0);
    fcntl(sv[1], F_SETFL, flags | O_NONBLOCK);

    char buf[16] = {0};
    int n = read(sv[1], buf, sizeof(buf));
    printf("nonblocking read on empty: n=%d\n", n);

    write(sv[0], "X", 1);
    n = read(sv[1], buf, sizeof(buf));
    printf("nonblocking read after write: n=%d buf='%s'\n", n, buf);

    close(sv[0]);
    close(sv[1]);
}

static void test_syscalls_presence(void)
{
    struct msghdr msg = {0};
    int sv[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0) {
        ssize_t n = sendmsg(sv[0], &msg, 0);
        printf("sendmsg returned %d\n", (int)n);

        n = recvmsg(sv[1], &msg, 0);
        printf("recvmsg returned %d\n", (int)n);
    }

    int fd = memfd_create("test", 0);
    printf("memfd_create returned %d\n", fd);
}

static void test_memfd(void)
{
    printf("Testing memfd_create...\n");
    int fd = memfd_create("test", 0);
    printf("memfd_create returned %d\n", fd);

    if (fd >= 0) {
        const char *msg = "Hello memfd";
        write(fd, msg, 11);

        lseek(fd, 0, 0); // your existing lseek syscall

        char buf[16] = {0};
        int n = read(fd, buf, sizeof(buf));
        printf("memfd read returned %d, buf='%s'\n", n, buf);

        close(fd);
    }
}

static void test_ftruncate_memfd(void)
{
    printf("Testing ftruncate on memfd...\n");

    int fd = memfd_create("demo", 0);
    printf("memfd fd=%d\n", fd);

    ftruncate(fd, 4096);   // grow to 4 KB

    lseek(fd, 0, 0);

    char buf[16] = {0};
    int n = read(fd, buf, sizeof(buf));
    printf("read after truncate returned %d\n", n);

    close(fd);
}
static void test_memfd_mmap(void)
{
    printf("Testing memfd + mmap...\n");

    int fd = memfd_create("wayland-test", 0);
    printf("memfd fd=%d\n", fd);

    size_t size = 4096;
    ftruncate(fd, size);

    // printf("DEBUG: calling mmap: size=%zu, fd=%d, offset=%d\n",
    //    size, fd, (long)0);

    printf("DEBUG: calling mmap\n");

    void *p = mmap(NULL, size,
                   PROT_READ | PROT_WRITE,
                   MAP_SHARED,   // <-- important
                   fd,
                   0);
    printf("mmap returned %p\n", p);

    if (p == MAP_FAILED) {
        printf("mmap failed\n");
        close(fd);
        return;
    }

    // Write into the mapping
    char *c = (char *)p;
    c[0] = 'Z';
    c[1] = 'Y';
    c[2] = 'X';
    c[3] = '\0';

    printf("memfd mmap content: '%s'\n", c);

    // Optionally: read via read() to see what you get
    lseek(fd, 0, SEEK_SET);
    char buf[8] = {0};
    int n = read(fd, buf, sizeof(buf));
    printf("read after mmap returned %d, buf='%s'\n", n, buf);

    close(fd);
}

static void test_raw_socketpair(void)
{
    // int sv[2] = {-1, -1};

    // long ret = syscall6(SYS_socketpair,
    //                     AF_UNIX,
    //                     SOCK_STREAM,
    //                     0,
    //                     (long)(uintptr_t)sv,
    //                     0,
    //                     0);


    int sv[2];
    printf("raw: sv at %p\n", (void*)sv);
    long ret = syscall6(SYS_socketpair,
                        AF_UNIX,
                        SOCK_STREAM,
                        0,
                        (long)(uintptr_t)sv,
                        0,
                        0);



    printf("raw: syscall6 ret=%ld, sv[0]=%d sv[1]=%d\n", ret, sv[0], sv[1]);
}


void testAllSyscalls()
{

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


    long r_test = syscall6(SYS_test,
                 0x11,
                 0x22,
                 0x33,
                 0x4444555566667777ULL, 0, 0);

    printf("syscall6(SYS_test,...) = %ld\n", r_test);

    printf("Testing socketpair...\n");
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0) {
        printf("socketpair created: %d, %d\n", sv[0], sv[1]);

        write(sv[0], "Hello SP!", 9);

        char buf[32] = {0};
        int n = read(sv[1], buf, 31);

        printf("socketpair read returned %d, buf='%s'\n", n, buf);

        close(sv[0]);
        close(sv[1]);
    }

    
    printf("Testing test_raw_socketpair...\n");

    test_raw_socketpair();

    printf("About to call test_socketpair_poll\n");
    test_socketpair_poll();

    printf("Testing test_socketpair_nonblock\n");
    test_socketpair_nonblock();

    printf("Testing test_syscalls_presence\n");
    test_syscalls_presence();
// __asm__ volatile("int3"); 
    printf("Testing test_memfd\n");
    test_memfd();

    printf("Testing test_ftruncate_memfd\n");
    test_ftruncate_memfd();

    printf("Testing test_memfd_mmap\n");

    test_memfd_mmap();
}

int test_client_server_socket(void)
{
    klog("[Userspace] Testing socket..\n");

    // 1) server: create, bind, listen
    int s = socket(AF_UNIX, SOCK_STREAM, 0);
    printf("[Userspace] socket s=%d\n", s);

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "/wayland-0");

    klog("[Userspace] Before bind\n");
    bind(s, (struct sockaddr *)&addr, sizeof(addr));
    klog("[Userspace] After bind\n");

    listen(s, 16);
    klog("[Userspace] server: listening on /wayland-0\n");

    // 2) client: create + connect
    int c = socket(AF_UNIX, SOCK_STREAM, 0);
    klog("[Userspace] client: connecting...\n");

    int r = connect(c, (struct sockaddr *)&addr, sizeof(addr));
    if (r < 0) {
        klog("[Userspace] connect FAILED\n");
        close(c);
        return -1;
    }

    klog("[Userspace] connect OK\n");
    klog_hex("connect_ret", r);

    // 3) server: accept client
    int as = accept(s, NULL, NULL);
    if (as < 0) {
        klog("[Userspace] accept FAILED\n");
        klog_hex("accept_ret", as);
        close(c);
        close(s);
        return -1;
    }

    klog("[Userspace] accept OK\n");
    klog_hex("accept_ret", as);

    // 4) server: write to accepted socket
    const char smsg[] = "hello from server\n";
    write(as, smsg, sizeof(smsg)-1);

    // DO NOT CLOSE 'as' — we want to return it alive
    // close(as);   <-- removed

    // 5) client: read from its socket
    char buf[128];
    int n = read(c, buf, sizeof(buf));
    printf("[Userspace] client: read returned %d\n", n);
    if (n > 0) {
        write(1, buf, n);
    }

    // Close listening socket
    close(s);

    // Optionally close client socket
    // close(c);

    // Return the accepted socket (still open)
    return as;
}


void testPollWakesWhenDataArrives() 
{
    printf("=== testPollWakesWhenDataArrives ===\n");

    int s = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "/test-sock");

    bind(s, (struct sockaddr*)&addr, sizeof(addr));
    listen(s, 1);

    printf("Server: waiting for client...\n");

    struct pollfd p;
    p.fd = s;
    p.events = POLLIN;

    // Spawn client
    int c = socket(AF_UNIX, SOCK_STREAM, 0);
    connect(c, (struct sockaddr*)&addr, sizeof(addr));

    // Now poll should wake because a connection is pending
    int ret = poll(&p, 1, -1);
    printf("poll ret=%d revents=%x\n", ret, p.revents);

    int as = accept(s, NULL, NULL);
    printf("accept returned %d\n", as);

    close(as);
    close(c);
    close(s);
}


void testPoll()
{
    struct pollfd p;
    p.fd = 0;        // stdin
    p.events = POLLIN;

    int ret = poll(&p, 1, 0);
    printf("poll ret=%d revents=%x\n", ret, p.revents);
}

void testPollWithTimeout()
{
    klog("testPollWithTimeout start\n");

    struct pollfd p;
    p.fd = 0;
    p.events = POLLIN;

    uint64_t start = get_time_ms();
    printf("testPollWithTimeout start=%d \n", start);

    int ret = poll(&p, 1, 2000);  // 2 seconds
    printf("testPollWithTimeout ret=%d \n", ret);

    uint64_t end = get_time_ms();
    printf("testPollWithTimeout end=%d \n", end);

    printf("poll ret=%d elapsed=%ld ms\n", ret, (long)(end - start));
}

void testPollOnUNIXSocket()
{
    printf("=== testPollOnUNIXSocket ===\n");

    int s = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "/test-sock2");

    bind(s, (struct sockaddr*)&addr, sizeof(addr));
    listen(s, 1);

    // Create client
    int c = socket(AF_UNIX, SOCK_STREAM, 0);
    connect(c, (struct sockaddr*)&addr, sizeof(addr));

    int as = accept(s, NULL, NULL);

    // Write something from client → server should wake
    write(c, "X", 1);

    struct pollfd p;
    p.fd = as;
    p.events = POLLIN;

    int ret = poll(&p, 1, -1);
    printf("poll ret=%d revents=%x\n", ret, p.revents);

    char buf[8];
    int n = read(as, buf, sizeof(buf));
    printf("server read %d bytes: '%c'\n", n, buf[0]);

    close(as);
    close(c);
    close(s);
}

void testPollOnClientSocket(int fd)
{
    struct pollfd p;
    p.fd = fd;
    p.events = POLLIN;

    printf("poll on client fd=%d...\n", fd);
    int ret = poll(&p, 1, 0);
    printf("poll ret=%d revents=%x\n", ret, p.revents);
}

int testSocket()
{
    int s1 = socket(AF_UNIX, SOCK_STREAM, 0);
    int s2 = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "/test-sock");

    printf("bind1 = %d\n", bind(s1, (struct sockaddr*)&addr, sizeof(addr)));
    printf("bind2 = %d\n", bind(s2, (struct sockaddr*)&addr, sizeof(addr)));

}

void testSCMRights()
{
    printf("=== testSCMRights ===\n");

    // 1. Create memfd
    int memfd = memfd_create("demo", 0);
    if (memfd < 0) {
        printf("memfd_create failed\n");
        return;
    }

    // Write something into it
    const char *msg = "HELLO-FD";
    write(memfd, msg, strlen(msg));

    // 2. Create UNIX socketpair
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
        printf("socketpair failed\n");
        return;
    }

    // 3. Prepare sendmsg with SCM_RIGHTS
    struct iovec iov;
    iov.iov_base = "X";
    iov.iov_len  = 1;

    char control[CMSG_SPACE(sizeof(int))];
    memset(control, 0, sizeof(control));

    struct msghdr msg_send = {0};
    msg_send.msg_iov = &iov;
    msg_send.msg_iovlen = 1;
    msg_send.msg_control = control;
    msg_send.msg_controllen = sizeof(control);

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg_send);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type  = SCM_RIGHTS;
    cmsg->cmsg_len   = CMSG_LEN(sizeof(int));

    memcpy(CMSG_DATA(cmsg), &memfd, sizeof(int));

    // 4. Send the fd
    if (sendmsg(sv[0], &msg_send, 0) < 0) {
        printf("sendmsg failed\n");
        return;
    }

    // 5. Prepare recvmsg
    char buf[1];
    struct iovec riov;
    riov.iov_base = buf;
    riov.iov_len  = 1;

    char rcontrol[CMSG_SPACE(sizeof(int))];
    memset(rcontrol, 0, sizeof(rcontrol));

    struct msghdr msg_recv = {0};
    msg_recv.msg_iov = &riov;
    msg_recv.msg_iovlen = 1;
    msg_recv.msg_control = rcontrol;
    msg_recv.msg_controllen = sizeof(rcontrol);

    // 6. Receive
    if (recvmsg(sv[1], &msg_recv, 0) < 0) {
        printf("recvmsg failed\n");
        return;
    }

    struct cmsghdr *rc = CMSG_FIRSTHDR(&msg_recv);
    if (!rc || rc->cmsg_type != SCM_RIGHTS) {
        printf("No FD received\n");
        return;
    }

    int received_fd;
    memcpy(&received_fd, CMSG_DATA(rc), sizeof(int));

    printf("Received FD = %d\n", received_fd);

    // 7. mmap the received fd
    void *map = mmap(NULL, 4096, PROT_READ, MAP_SHARED, received_fd, 0);
    if (map == MAP_FAILED) {
        printf("mmap failed\n");
        return;
    }

    printf("Mapped contents: '%s'\n", (char*)map);

    munmap(map, 4096);
    close(received_fd);
    close(memfd);
    close(sv[0]);
    close(sv[1]);
}

void testClientSocketPool()
{
    long ret = syscall6(SYS_test, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66);
    printf("[Userspace] syscall6 early test ret=%ld\n", ret);

    const char msg[] = "[Userspace] Hello from SYSCALL userland!\n";
    syscall6(SYS_write, 1, (long)msg, sizeof(msg)-1, 0, 0, 0);

    int cfd = test_client_server_socket();
    testPollOnClientSocket(cfd);

    testPoll() ;
    testPollWithTimeout();
    testPollWakesWhenDataArrives();
    testPollOnUNIXSocket();

    testSocket();

    testSCMRights();
}

void start_compositor() 
{
    char *argv[] = { "/bin/comp", NULL };
    // char *argv[] = { "/bin/c-test", NULL };
    char *envp[] = { NULL };

    execve(argv[0], argv, envp);

    klog("execve failed for compositor\n");
}

int main()
{
    klog("[Userspace] Hello from userspace from klog from main!\n");
    printf("[Userspace] Hello from userspace from printf!\n");

    syscall6(SYS_test, 11, 22, 33, 44, 55, 66);

    // testClientSocketPool();


    // printf("Hello from /bin/init!\n");

    // // char *argv[] = { "test", NULL };
    // // execve("/bin/test", argv, NULL);

    // // // If execve fails:
    // // printf("execve /bin/test failed\n");


    start_compositor();

    // If compositor exits, keep init alive
    while (1)
    {

    }

    printf("[Userspace] About to call SYS_exit via SYSCALL\n");
    syscall6(SYS_exit, 0, 0, 0, 0, 0, 0);
    return 0;
}