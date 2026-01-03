// syscall_numbers.h
#pragma once

enum {
    SYS_test   = 0,
    SYS_write  = 1,
    SYS_exit   = 2,
    SYS_open   = 3,
    SYS_read   = 4,
    SYS_close  = 5,
    // later: SYS_mmap, SYS_poll, SYS_socket, ...
};
