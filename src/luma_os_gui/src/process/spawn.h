#include <sys/syscall.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

long spawn_path(const char* path);