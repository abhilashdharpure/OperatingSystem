// process.c
#include <hal/process.h>

Process *process_create(const char *name) {
    static Process dummy;
    (void)name;
    // pd/cr3 will be filled by exec_elf_mem, so no need to init here yet.
    return &dummy;
}

uintptr_t process_setup_stack(Process *p) {
    (void)p;
    return 0x800000; // fake stack pointer
}
