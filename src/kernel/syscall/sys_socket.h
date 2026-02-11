#pragma once

#include <stdint.h>
#include <types.h>
#include <unix_socket.h>

uint64_t sys_sp_sendmsg(uint64_t fd_arg, uint64_t msg_ptr_arg, uint64_t flags_arg);
uint64_t sys_sp_recvmsg(uint64_t fd_arg, uint64_t msg_ptr_arg, uint64_t flags_arg);
uint64_t sys_socket(uint64_t domain, uint64_t type, uint64_t protocol);
uint64_t sys_bind(uint64_t fd, uint64_t addr_ptr, uint64_t addrlen);
uint64_t sys_listen(uint64_t fd, uint64_t backlog);
uint64_t sys_connect(uint64_t fd, uint64_t addr_ptr, uint64_t addrlen);
uint64_t sys_accept(uint64_t fd, uint64_t addr_ptr, uint64_t addrlen_ptr);
