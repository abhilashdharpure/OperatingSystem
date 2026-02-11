#pragma once
#include <stdint.h>
#include <types.h>

#define AF_UNIX     1
#define SOCK_STREAM 1
#define SOL_SOCKET 1
#define SCM_RIGHTS 1


typedef unsigned int socklen_t;

struct sockaddr {
    uint16_t sa_family;
    char     sa_data[14];
};

