#pragma once
#include <sys/syscall.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <sys/wait.h>
#include <errno.h>


long spawn_path(const char* path);

// // flags 0: the parent must waitpid() later. Use 1 (SPAWN_DETACH) to let the kernel reap it.
// long luma_spawn(const std::string& path, const std::vector<std::string>& args = {}, int flags = 1);


#ifndef SYS_spawn
#define SYS_spawn 0x1001
#endif

// flags 0: the parent must waitpid() later. Use 1 (SPAWN_DETACH) to let the kernel reap it.
inline long luma_spawn(const std::string& path, const std::vector<std::string>& args = {}, int flags = 1)
{
    std::vector<char*> argv;
    argv.push_back((char*)path.c_str());
    for (auto& a : args) argv.push_back((char*)a.c_str());
    argv.push_back(nullptr);
    char* envp[] = { (char*)"XDG_RUNTIME_DIR=/tmp", (char*)"WAYLAND_DISPLAY=wayland-0", nullptr };
    return syscall(SYS_spawn, path.c_str(), argv.data(), envp, flags);
}