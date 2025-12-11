#pragma once
#include <stdint.h>
#include <hal/process.h>


void scheduler_add(Process *p);
void scheduler_switch_to_user(Process *p);
