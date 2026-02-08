#pragma once
#include <stddef.h>
#include "stdint.h"

void* memcpy(void* dst, const void* src, uint16_t num);
void* memset(void* ptr, int value, uint16_t num);
int memcmp(const void* ptr1, const void* ptr2, uint16_t num);

const char* strchr(const char* str, char chr);
char* strcpy(char* dst, const char* src);
unsigned strlen(const char* str);
size_t strnlen(const char *s, size_t maxlen); 
int strcmp(const char* a, const char* b);
int strncmp(const char *s1, const char *s2, size_t n);
char* strncpy(char* dst, const char* src, size_t n);
void *memmove(void *dest, const void *src, size_t n);

wchar_t* utf16_to_codepoint(wchar_t* string, int* codepoint);
char* codepoint_to_utf8(int codepoint, char* stringOutput);
char* kstrdup(const char* src);
int toupper(int c);
char* strcat(char* dst, const char* src);
