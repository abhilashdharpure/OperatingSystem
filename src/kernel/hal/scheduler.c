// scheduler.c
#include <hal/scheduler.h>
void scheduler_add(Process *p) { /* no-op */ }
void scheduler_switch_to_user(Process *p) {
    // for now just panic to show we reached here
    panic("switch_to_user not implemented");
}
