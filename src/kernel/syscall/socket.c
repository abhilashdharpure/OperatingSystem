#include <syscall/socket.h>
#include <syscall/un.h>
#include <hal/socketpair.h>
#include <hal/file.h>
#include "unix_socket.h"
#include "sys_socket.h"
#include "errno.h"
#include "syscall/uio.h"
#include "syscall/cmsg.h"

#define MAX_UNIX_SOCKS 64
static unix_socket_t *g_unix_socks[MAX_UNIX_SOCKS];
