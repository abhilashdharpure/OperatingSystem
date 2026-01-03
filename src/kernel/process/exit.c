#include <debug.h>

__attribute__((noreturn))
void sys_exit(int code)
{
    log_info("EXIT", "userspace exited with code %d", code);

    /* 
     * TEMPORARY BEHAVIOR:
     * Halt CPU so we never SYSRET into dead userspace.
     * This is correct until a scheduler exists.
     */
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}
