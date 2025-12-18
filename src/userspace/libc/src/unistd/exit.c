#include <unistd.h>
#include <syscall.h>

void _exit(int code) {
    syscall_exit(code);
    while (1) { }
}

void exit(int code) {
    _exit(code);
}
