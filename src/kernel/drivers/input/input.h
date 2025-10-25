#pragma once

#include "stdint.h"
#include "arch/i686/events/input_event.h"
#include <util/string.h>


typedef struct __attribute__((packed)) {
    uint64_t time;        // when the event happened // microseconds since boot (optional)
    uint16_t type;        // EV_KEY, EV_REL, etc.
    uint16_t code;        // KEY_A, REL_X, BTN_LEFT, etc.
    int32_t  value;       // meaning depends on type+code
} InputEvent;

// typedef struct  {
//     uint64_t time;        // when the event happened // microseconds since boot (optional)
//     uint16_t type;        // EV_KEY, EV_REL, etc.
//     uint16_t code;        // KEY_A, REL_X, BTN_LEFT, etc.
//     int32_t  value;       // meaning depends on type+code
// } InputEvent;

#define INPUT_BUFFER_SIZE 64

struct input_device {
    char name[32];
    InputEvent buffer[INPUT_BUFFER_SIZE];
    int head;
    int tail;
    int count;
    struct file_operations *fops;
};

// struct file_operations input_fops = {
//     .open = input_open,
//     .read = input_read,
// };

// typedef struct file_operations {
//     .open = input_open,
//     .read = input_read,
// } input_fops;

void input_push_event(struct input_device *dev, InputEvent* inputEvent);
int input_pop_event(struct input_device *dev, InputEvent *out);

// size_t input_read(struct file *f, void *buf, size_t count);
// int input_open(struct file *f);

void register_input_device(struct input_device *dev, const char *name);
void debug_poll_input(struct input_device *dev);

int8_t file(const char *fileName);
