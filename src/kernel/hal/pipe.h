#pragma once
#include <stddef.h>
#include <stdint.h>

#define PIPE_BUF_SIZE 4096

typedef struct pipe {
    uint8_t  buffer[PIPE_BUF_SIZE];
    size_t   read_pos;
    size_t   write_pos;
    size_t   count;       // bytes currently stored
    int      refcount;    // number of file endpoints pointing here
} pipe_t;

struct file_operations *pipe_get_fops(void);
