#pragma once

#define AF_UNIX 1
#define SOCK_STREAM 1

int socketpair(int domain, int type, int protocol, int sv[2]);
