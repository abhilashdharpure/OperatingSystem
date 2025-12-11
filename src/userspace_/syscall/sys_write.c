#include <stdint.h>
#include "debug.h"
#include "process.h"

#define STDOUT 1
#define STDERR 2

int sys_write(int fd, const char *user_buf, uint32_t len)
{
    if (fd != STDOUT && fd != STDERR)
        return -1;

    char c;

    for (uint32_t i = 0; i < len; i++) {
        if (!copy_from_user(&c, user_buf + i, 1))
            return -1;

        debug_putchar(c);   // serial or VGA
    }

    return len;
}
