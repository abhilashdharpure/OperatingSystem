#include "kmalloc.h"

static uint8_t *heap_ptr = (uint8_t *)KERNEL_HEAP_START;
static size_t used = 0;

void *kmalloc(size_t size)
{
    if (size == 0)
        return NULL;

    /* simple bump allocator; return NULL on OOM */
    if (used + size > KERNEL_HEAP_SIZE)
        return NULL;

    void *addr = heap_ptr + used;
    used += size;
    return addr;
}

void kfree(void *ptr)
{
    (void)ptr; /* no-op for bump allocator */
}