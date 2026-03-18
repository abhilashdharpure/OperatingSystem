# pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MAX_EXEC_ARGS     16
#define MAX_EXEC_ARG_LEN  256


bool copy_from_user_byte(uint8_t *out, const uint8_t *user_ptr);
bool copy_from_user_ptr(void *out, const void *user_ptr);
int copy_from_user(void *dst, const void *src, size_t n);
void *read_entire_file(const char *path, size_t *out_size);