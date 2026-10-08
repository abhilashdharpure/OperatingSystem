#pragma once

#include <unistd.h>
#include "defines.h"

typedef struct {
    int64_t tv_sec;
    int64_t tv_usec;
} TimeVal;

typedef struct {
    TimeVal time;
    uint16_t type;
    uint16_t code;
    int32_t value;
} InputEvent;
