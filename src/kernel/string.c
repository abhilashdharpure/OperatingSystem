#include "string.h"
#include <stdint.h>
#include <stddef.h>
#include "kmalloc.h"
#include <stdio.h>

void* memcpy(void* dst, const void* src, uint16_t num)
{
    uint8_t* u8Dst = (uint8_t *)dst;
    const uint8_t* u8Src = (const uint8_t *)src;

    for (uint16_t i = 0; i < num; i++)
        u8Dst[i] = u8Src[i];

    return dst;
}

void * memset(void * ptr, int value, uint16_t num)
{
    uint8_t* u8Ptr = (uint8_t *)ptr;

    for (uint16_t i = 0; i < num; i++)
        u8Ptr[i] = (uint8_t)value;

    return ptr;
}

int memcmp(const void* ptr1, const void* ptr2, uint16_t num)
{
    const uint8_t* u8Ptr1 = (const uint8_t *)ptr1;
    const uint8_t* u8Ptr2 = (const uint8_t *)ptr2;

    for (uint16_t i = 0; i < num; i++)
        if (u8Ptr1[i] != u8Ptr2[i])
            return 1;

    return 0;
}


const char* strchr(const char* str, char chr)
{
    if (str == NULL)
        return NULL;

    while (*str)
    {
        if (*str == chr)
            return str;

        ++str;
    }

    return NULL;
}

char* strcpy(char* dst, const char* src)
{
    char* origDst = dst;

    if (dst == NULL)
        return NULL;

    if (src == NULL)
    {
        *dst = '\0';
        return dst;
    }

    while (*src)
    {
        *dst = *src;
        ++src;
        ++dst;
    }
    
    *dst = '\0';
    return origDst;
}

unsigned strlen(const char* str)
{
    unsigned len = 0;
    while (*str)
    {
        ++len;
        ++str;
    }

    return len;
}
size_t strnlen(const char *s, size_t maxlen)
{
    size_t i = 0;
    while (i < maxlen && s[i])
        i++;
    return i;
}

int strcmp(const char* a, const char* b)
{
    if (a == NULL && b == NULL)
        return 0;

    if (a == NULL || b == NULL)
        return -1;

    while (*a && *b && *a == *b)
    {
        ++a;
        ++b;
    }
    return (*a) - (*b);
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        unsigned char c1 = (unsigned char)s1[i];
        unsigned char c2 = (unsigned char)s2[i];
        if (c1 != c2) return c1 - c2;
        if (c1 == '\0') return 0;
    }
    return 0;
}

char* strncpy(char* dst, const char* src, size_t n)
{
    size_t i = 0;
    for (; i < n && src[i]; i++)
        dst[i] = src[i];

    for (; i < n; i++)
        dst[i] = '\0';

    return dst;
}

void *memmove(void *dest, const void *src, size_t n)
{
    unsigned char *d = dest;
    const unsigned char *s = src;

    if (d == s || n == 0)
        return dest;

    if (d < s) {
        // forward copy
        for (size_t i = 0; i < n; i++)
            d[i] = s[i];
    } else {
        // backward copy
        for (size_t i = n; i != 0; i--)
            d[i - 1] = s[i - 1];
    }

    return dest;
}


wchar_t* utf16_to_codepoint(wchar_t* string, int* codepoint)
{
    int c1 = *string;
    ++string;

    if (c1 >= 0xd800 && c1 < 0xdc00) {
        int c2 = *string;
        ++string;
        *codepoint = ((c1 & 0x3ff) << 10) + (c2 & 0x3ff) + 0x10000;
    }
    *codepoint = c1;

    return string;
}

/* Encoding
   The following byte sequences are used to represent a
   character.  The sequence to be used depends on the UCS code
   number of the character:

   0x00000000 - 0x0000007F:
       0xxxxxxx

   0x00000080 - 0x000007FF:
       110xxxxx 10xxxxxx

   0x00000800 - 0x0000FFFF:
       1110xxxx 10xxxxxx 10xxxxxx

   0x00010000 - 0x001FFFFF:
       11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

   [... removed obsolete five and six byte forms ...]

   The xxx bit positions are filled with the bits of the
   character code number in binary representation.  Only the
   shortest possible multibyte sequence which can represent the
   code number of the character can be used.

   The UCS code values 0xd800–0xdfff (UTF-16 surrogates) as well
   as 0xfffe and 0xffff (UCS noncharacters) should not appear in
   conforming UTF-8 streams.
*/

char* codepoint_to_utf8(int codepoint, char* stringOutput)
{
    if (codepoint <= 0x7F) {
        *stringOutput = (char)codepoint;
    }
    else if (codepoint <= 0x7FF) {
        *stringOutput++ = 0xC0 | ((codepoint >> 6) & 0x1F);
        *stringOutput++ = 0x80 | (codepoint & 0x3F);
    }
    else if (codepoint <= 0xFFFF) {
        *stringOutput++ = 0xE0 | ((codepoint >> 12) & 0xF);
        *stringOutput++ = 0x80 | ((codepoint >> 6) & 0x3F);
        *stringOutput++ = 0x80 | (codepoint & 0x3F);
    }
    else if (codepoint <= 0x1FFFFF) {
        *stringOutput++ = 0xF0 | ((codepoint >> 18) & 0x7);
        *stringOutput++ = 0x80 | ((codepoint >> 12) & 0x3F);
        *stringOutput++ = 0x80 | ((codepoint >> 6) & 0x3F);
        *stringOutput++ = 0x80 | (codepoint & 0x3F);
    }
    return stringOutput;
}

char* kstrdup(const char* src)
{
    if (!src)
        return NULL;

    size_t len = strlen(src) + 1;
    char* dst = kmalloc(len);
    if (!dst)
        return NULL;

    memcpy(dst, src, len);
    return dst;
}

static void itoa_dec(int value, char *buf)
{
    char tmp[16];
    int pos = 0;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    int negative = value < 0;
    if (negative) value = -value;

    while (value > 0) {
        tmp[pos++] = '0' + (value % 10);
        value /= 10;
    }

    int idx = 0;
    if (negative) buf[idx++] = '-';

    while (pos--)
        buf[idx++] = tmp[pos];

    buf[idx] = 0;
}

static void itoa_hex(unsigned value, char *buf)
{
    const char* hex = "0123456789ABCDEF";
    int pos = 0;
    char tmp[16];

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (value) {
        tmp[pos++] = hex[value & 0xF];
        value >>= 4;
    }

    int idx = 0;
    while (pos--)
        buf[idx++] = tmp[pos];

    buf[idx] = 0;
}

int snprintf(char* out, size_t size, const char* fmt, ...)
{
    if (size == 0) return 0;

    va_list args;
    va_start(args, fmt);

    size_t pos = 0;
    char numbuf[32];

    while (*fmt && pos < size - 1) {
        if (*fmt != '%') {
            out[pos++] = *fmt++;
            continue;
        }

        fmt++;  // skip %

        switch (*fmt) {
            case 's': {
                const char *s = va_arg(args, const char*);
                while (*s && pos < size - 1)
                    out[pos++] = *s++;
                break;
            }
            case 'd': {
                int v = va_arg(args, int);
                itoa_dec(v, numbuf);
                for (char* p = numbuf; *p && pos < size - 1; p++)
                    out[pos++] = *p;
                break;
            }
            case 'x': {
                unsigned v = va_arg(args, unsigned);
                itoa_hex(v, numbuf);
                for (char* p = numbuf; *p && pos < size - 1; p++)
                    out[pos++] = *p;
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                out[pos++] = c;
                break;
            }
            default:
                out[pos++] = '%';
                out[pos++] = *fmt;
                break;
        }

        fmt++;
    }

    out[pos] = '\0';
    va_end(args);
    return pos;
}
int toupper(int c)
{
    if (c >= 'a' && c <= 'z')
        return c - 32;
    return c;
}

char* strcat(char* dst, const char* src)
{
    char* p = dst + strlen(dst);
    while (*src)
        *p++ = *src++;
    *p = '\0';
    return dst;
}