#ifndef MEMFILE_H
#define MEMFILE_H

#include <stdint.h>
#include "hal/vfs.h"

typedef struct {
    uint8_t *data;
    uint32_t size;
} memfile_t;

typedef struct {
    memfile_t *mf;
    uint32_t offset;
} memfile_handle_t;

extern struct file_operations memfile_fops;

#endif
