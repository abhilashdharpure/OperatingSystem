#include <syscall.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include "uprintf.h"

#include <sys/execve.h>

#define SYS_spawn   0x1001
#define SPAWN_DETACH 1

static void spawn_client(const char* path)
{
    char* argv[] = { (char*)path, NULL };
    char* envp[] = { (char*)"XDG_RUNTIME_DIR=/tmp",
                     (char*)"WAYLAND_DISPLAY=wayland-0", NULL };
    long pid = syscall(SYS_spawn, path, argv, envp, SPAWN_DETACH);
    printf("[LumaCompositor] spawned %s pid=%ld\n", path, pid);
}

static long spawn_path(const char* path)
{
    char* argv[] = { (char*)path, NULL };
    char* envp[] = { (char*)"XDG_RUNTIME_DIR=/tmp",
                     (char*)"WAYLAND_DISPLAY=wayland-0", NULL };
    return syscall(SYS_spawn, path, argv, envp, 0);   /* 0: init reaps via wait4 */
}

void start_compositor() 
{
    char *argv[] = { "/bin/comp", NULL };
    char *envp[] = { NULL };

    execve(argv[0], argv, envp);

    klog("execve failed for compositor\n");
}

void start_client() 
{
    klog("Starting client using Spawn..\n");

    spawn_client("/bin/client");

}

int main()
{
    klog("[Userspace] init start\n");

    long comp_pid = spawn_path("/bin/comp");
    klog("spawned compositor\n");

    /* give the compositor time to create /tmp/wayland-0 before the client connects.
       Simplest: yield a few times. The compositor runs until it blocks in epoll_wait. */
    for (int i = 0; i < 50; i++) syscall(24 /* sched_yield */);

    long client_pid = spawn_path("/bin/client");
    klog("spawned client\n");

    /* init reaps children forever */
    int st;
    for (;;) {
        long r = syscall(61 /* wait4 */, -1, &st, 0, 0);
        if (r < 0) syscall(24);              /* no children: just yield */
    }
    (void)comp_pid; (void)client_pid;
}

// int main()
// {
//     klog("[Userspace] Hello from userspace from klog from main!\n");
//     // printf("[Userspace] Hello from userspace from printf!\n");
//     // printf("[Userspace] Issue after first printf, this will not print\n");
//     klog("[Userspace] KLog is still fine..!\n");


//     syscall6(SYS_test, 11, 22, 33, 44, 55, 66);

//     start_compositor();

//     start_client();

//     // If compositor exits, keep init alive
//     while (1)
//     {

//     }

//     printf("[Userspace] About to call SYS_exit via SYSCALL\n");
//     syscall6(SYS_exit, 0, 0, 0, 0, 0, 0);
//     return 0;
// }
