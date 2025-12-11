// process.c
#include <hal/process.h>
Process *process_create(const char *name) {
    static Process dummy;
    return &dummy;
}
uintptr_t process_setup_stack(Process *p) {
    return 0x800000; // fake stack pointer
}
