#pragma once
#include <stdint.h>
#include <types.h>

enum {
    SYS_write           = 1,
    SYS_exit            = 60,
    SYS_open            = 2,
    SYS_read            = 0,
    SYS_close           = 3,
    SYS_mmap            = 9,
    SYS_munmap          = 11,
    SYS_mprotect        = 10,
    SYS_brk             = 12,
    SYS_ioctl           = 16,
    SYS_writev          = 20,
    SYS_poll            = 7,
    SYS_stat            = 4,   // or newfstatat/fstatat64 depending on what you wire
    SYS_fstat           = 5,
    SYS_lseek           = 8,
    SYS_getdents        = 78,  // or getdents64 = 217
    SYS_dup             = 32,
    SYS_dup2            = 33,
    SYS_fcntl           = 72,  // or fcntl64 = 72 on x86_64
    SYS_pipe            = 22,
    SYS_klog            = 0x1000, // your private one, keep out of Linux range
    SYS_clock_gettime   = 228,
    SYS_nanosleep       = 35,
    SYS_socketpair      = 53,
    SYS_sendmsg         = 46,
    SYS_recvmsg         = 47,
    SYS_memfd_create    = 319,
    SYS_ftruncate       = 77,
    SYS_getpid          = 39,
    SYS_getppid         = 110,
    SYS_uname           = 63,
    SYS_getcwd          = 79,
    SYS_madvise         = 28,
    SYS_set_tid_address = 218,
    SYS_prlimit64       = 302,
    SYS_getrandom       = 318,
    SYS_exit_group      = 231,
    SYS_arch_prctl      = 158,

    SYS_test            = 999,

    SYS_socket          = 41,
    SYS_connect         = 42,
    SYS_accept          = 43,
    SYS_bind            = 49,
    SYS_listen          = 50,

};

long syscall(long n, ...);
long syscall6(long n, long a, long b, long c, long d, long e, long f);

long syscall1(long nr, long a0);
long syscall2(long nr, long a0, long a1);
long syscall3(long nr, long a0, long a1, long a2);
long syscall4(long nr, long a0, long a1, long a2, long a3);
long syscall5(long nr, long a0, long a1, long a2, long a3, long a4);