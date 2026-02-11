#pragma once
#include <stddef.h>
#include <stdint.h>

struct cmsghdr_k {
    size_t cmsg_len;
    int    cmsg_level;
    int    cmsg_type;
};

// Alignment helper
#define CMSG_ALIGN_K(len)   (((len) + sizeof(size_t) - 1) & ~(sizeof(size_t) - 1))

// Space for header + data
#define CMSG_LEN_K(len)     (CMSG_ALIGN_K(sizeof(struct cmsghdr_k)) + (len))
#define CMSG_SPACE_K(len)   (CMSG_ALIGN_K(sizeof(struct cmsghdr_k)) + CMSG_ALIGN_K(len))
