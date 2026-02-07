#pragma once
#include <syscall.h>
#include <stdint.h>
#include <types.h>

// Musl expects __NR_* or SYS_* macros.
// You can provide both for compatibility.

#define __NR_test            SYS_test
#define __NR_write           SYS_write
#define __NR_exit            SYS_exit
#define __NR_open            SYS_open
#define __NR_read            SYS_read
#define __NR_close           SYS_close
#define __NR_mmap            SYS_mmap
#define __NR_munmap          SYS_munmap
#define __NR_mprotect        SYS_mprotect
#define __NR_brk             SYS_brk
#define __NR_poll            SYS_poll
#define __NR_stat            SYS_stat
#define __NR_fstat           SYS_fstat
#define __NR_lseek           SYS_lseek
#define __NR_getdents        SYS_getdents
#define __NR_dup             SYS_dup
#define __NR_dup2            SYS_dup2
#define __NR_fcntl           SYS_fcntl
#define __NR_pipe            SYS_pipe
#define __NR_klog            SYS_klog
#define __NR_clock_gettime   SYS_clock_gettime
#define __NR_nanosleep       SYS_nanosleep
#define __NR_socketpair      SYS_socketpair
#define __NR_sendmsg         SYS_sendmsg
#define __NR_recvmsg         SYS_recvmsg
#define __NR_memfd_create    SYS_memfd_create
#define __NR_ftruncate       SYS_ftruncate


_Static_assert(SYS_test == 999, "SYS_test mismatch");
_Static_assert(SYS_write == 1, "SYS_write mismatch");
_Static_assert(SYS_exit == 60, "SYS_exit mismatch");
