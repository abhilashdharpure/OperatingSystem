#pragma once
#include <stdint.h>

#define NAME_MAX 100

struct dirent {
    uint32_t d_ino;
    uint8_t  d_type;           // 0=file,1=dir,2=device...
    char     d_name[NAME_MAX];
};

int getdents(const char *path, struct dirent *buf, int max_entries);