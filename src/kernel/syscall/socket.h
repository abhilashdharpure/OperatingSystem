#pragma once
#include <stdint.h>
#include <types.h>

#define AF_UNIX     1
#define SOCK_STREAM 1
#define SOL_SOCKET  1
#define SCM_RIGHTS  1
#define SO_ERROR    4
#define SO_PEERCRED 17
#define ENOPROTOOPT 92

typedef unsigned int socklen_t;

struct sockaddr {
    uint16_t sa_family;
    char     sa_data[14];
};

