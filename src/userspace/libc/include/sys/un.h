#pragma once

#include <stdint.h>
#include <types.h>

#define UNIX_PATH_MAX 108

struct sockaddr_un {
    uint16_t sun_family;          // AF_UNIX
    char     sun_path[UNIX_PATH_MAX];
};
