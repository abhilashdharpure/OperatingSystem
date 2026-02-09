#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct unix_socket
{
    bool listening;
    struct unix_socket *peer;

    char   buf[4096];
    size_t buf_len;

    struct unix_socket *pending[16];
    int pending_head;
    int pending_tail;

    char path[108];
} unix_socket_t;
