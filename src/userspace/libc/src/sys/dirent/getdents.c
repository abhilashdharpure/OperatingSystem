#include <dirent.h>
#include <syscall.h>

int getdents(const char *path, struct dirent *buf, int max_entries)
{
    long r = syscall(SYS_getdents,
                     (long)path,
                     (long)buf,
                     (long)max_entries,
                     0);
    return (int)r;
}
