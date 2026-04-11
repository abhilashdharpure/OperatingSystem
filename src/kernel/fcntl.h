#pragma once

#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_NONBLOCK  0x0400

#define F_DUPFD             0
#define F_GETFD             1
#define F_SETFD             2
#define F_GETFL             3
#define F_SETFL             4       
#define F_DUPFD_CLOEXEC     1030   // Linux x86_64 ABI