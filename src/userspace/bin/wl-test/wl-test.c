#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    printf("Hello from /bin/wl-test!\n");
    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = '%s'\n", i, argv[i]);
    }

    // later: open /dev/fb0, mmap it, draw pixels, etc.
    return 0;
}
