#pragma once

// Standard POSIX error numbers (minimal set)
#define EAGAIN      11
#define EWOULDBLOCK EAGAIN
#define EBADF       9
#define EINVAL      22
#define ENOMEM      12
#define EPIPE       32
#define ENOSYS      38
#define ENOBUFS     105
#define EMFILE 24 // Too many open files (per process)
#define ENFILE 23 // Too many open files in system (optional)

// errno is a global variable in simple libc
extern int errno;
