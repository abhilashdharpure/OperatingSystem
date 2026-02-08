#include <syscall/socket.h>
#include <syscall/un.h>
#include <hal/socketpair.h>
#include <hal/file.h>
#include "unix_socket.h"
#include "sys_socket.h"
#include "errno.h"

#define MAX_UNIX_SOCKS 64
static unix_socket_t *g_unix_socks[MAX_UNIX_SOCKS];

// static void unix_register_socket(unix_socket_t *us)
// {
//     for (int i = 0; i < MAX_UNIX_SOCKS; ++i) {
//         if (!g_unix_socks[i]) {
//             g_unix_socks[i] = us;
//             return;
//         }
//     }
// }

// static unix_socket_t *unix_find_by_path(const char *path)
// {
//     for (int i = 0; i < MAX_UNIX_SOCKS; ++i) {
//         unix_socket_t *us = g_unix_socks[i];
//         if (us && strcmp(us->path, path) == 0)
//             return us;
//     }
//     return NULL;
// }

// static int unix_sock_close(struct file *f)
// {
//     unix_socket_t *us = (unix_socket_t *)f->private_data;
//     // TODO: free peers, pending, etc. For now just free self.
//     if (us) {
//         kfree(us);
//         f->private_data = NULL;
//     }
//     return 0;
// }

