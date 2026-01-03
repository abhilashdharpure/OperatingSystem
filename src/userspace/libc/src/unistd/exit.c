#include <unistd.h>
#include <syscall.h>

void _exit(int code) {
    // syscall_exit(code);
     syscall(SYS_exit, (long)code, 0, 0);
    while (1) { }
}

void exit(int code) {
    _exit(code);
}
