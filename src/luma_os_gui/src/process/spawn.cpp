#include "spawn.h"


#define SYS_spawn   0x1001
#define SPAWN_DETACH 1

long spawn_path(const char* path)
{
    char* argv[] = { (char*)path, nullptr };
    char* envp[] = { (char*)"XDG_RUNTIME_DIR=/tmp",
                     (char*)"WAYLAND_DISPLAY=wayland-0", nullptr };
    return syscall(SYS_spawn, path, argv, envp, 0);     // 0 = the parent reaps via wait4
}