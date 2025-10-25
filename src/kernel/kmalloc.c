#include "kmalloc.h"

#define KERNEL_HEAP_START 0x1000000     // 16 MB mark (example)
#define KERNEL_HEAP_SIZE  (1024 * 1024) // 1 MB heap

static uint8_t* heap_ptr = (uint8_t*)KERNEL_HEAP_START;
static size_t used = 0;

void* kmalloc(size_t size)
{
    if (used + size > KERNEL_HEAP_SIZE)
        return NULL; // out of memory

    void* addr = heap_ptr + used;
    used += size;

    return addr;
}

void kfree(void* ptr)
{
    // No-op for bump allocator
    (void)ptr;
}