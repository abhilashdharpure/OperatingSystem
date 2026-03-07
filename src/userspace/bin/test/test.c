#include <stdio.h>
#include <unistd.h>
#include <sys/execve.h>

int main(int argc, char **argv)
{
    printf("Hello from /bin/test userspace!\n");

    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = '%s'\n", i, argv[i]);
    }

    char *argv2[] = { "wl-test", NULL };
    execve("/bin/wl-test", argv2, NULL);

    // execve failed
    printf("execve('/bin/wl-test') failed\n");
    return 1;
}
