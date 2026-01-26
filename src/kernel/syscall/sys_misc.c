#include "utsname.h"
#include <string.h>
#include "debug.h"
#include "errno.h"
#include "hal/process.h"

int sys_getpid(void) {
    return 1;   // single‑process system for now
}

int sys_getppid(void) {
    return 0;
}

int sys_uname(struct utsname *u) {
    strcpy(u->sysname, "myos");
    strcpy(u->nodename, "myos");
    strcpy(u->release, "0.1");
    strcpy(u->version, "0.1");
    strcpy(u->machine, "x86_64");
    return 0;
}

int sys_getcwd(char *buf, size_t size) {
    if (size < 2) return -EINVAL;
    buf[0] = '/';
    buf[1] = '\0';
    return 1;
}

int sys_madvise(void *addr, size_t len, int advice) {
    return 0;   // musl is fine with this
}

int sys_set_tid_address(int *tidptr) {
    return 1;
}

int sys_prlimit64(pid_t pid, int resource,
                  const struct rlimit *new_limit,
                  struct rlimit *old_limit) {
    return 0;
}

int sys_getrandom(void *buf, size_t len, unsigned flags) {
    // simple PRNG for now
    for (size_t i = 0; i < len; i++)
        ((unsigned char*)buf)[i] = (i * 37 + 13) & 0xFF;
    return len;
}

int sys_exit_group(int code) {
    // same as sys_exit for now
    sys_exit(code);
    return 0;
}
