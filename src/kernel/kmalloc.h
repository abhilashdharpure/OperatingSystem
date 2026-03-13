#pragma once

#include <stddef.h>
#include <stdint.h>

/* Kernel heap configuration (single authoritative place) */
#ifndef KERNEL_HEAP_START
#define KERNEL_HEAP_START 0x01000000UL   /* 16 MiB */
#endif

#ifndef KERNEL_HEAP_SIZE
#define KERNEL_HEAP_SIZE  (2 * 1024 * 1024UL) /* 2 MiB */
#endif

void *kmalloc(size_t size);
void kfree(void *ptr);
