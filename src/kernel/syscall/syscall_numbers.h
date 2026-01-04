// syscall_numbers.h
#pragma once

enum {
    SYS_test        = 0,
    SYS_write       = 1,
    SYS_exit        = 2,
    SYS_open        = 3,
    SYS_read        = 4,
    SYS_close       = 5,
    SYS_mmap        = 6,
    SYS_munmap      = 7,
    SYS_mprotect    = 8,
    SYS_brk         = 9,
    SYS_poll        = 10,
    SYS_stat        = 11,
    SYS_fstat       = 12,
    SYS_lseek       = 13,
    SYS_getdents    = 14,
    SYS_dup         = 15,
    SYS_dup2        = 16,
    SYS_fcntl       = 17,

    // later: SYS_socket, ..
};
