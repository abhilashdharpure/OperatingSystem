// process.c
#include <hal/process.h>
#include <debug.h>
#include "kmalloc.h"

Process *process_create(const char *name) {
    (void)name;

    Process *p = kmalloc(sizeof(Process));
    if (!p) {
        log_critical("PROC", "process_create: kmalloc(Process) failed");
        return NULL;
    }

    memset(p, 0, sizeof(Process));
    log_info("PROC", "process at %p", p);
    return p;
}
