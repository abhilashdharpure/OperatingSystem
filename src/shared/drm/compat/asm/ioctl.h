#pragma once
#ifndef _IOC
#define _IOC(dir, type, nr, size) (((dir) << 30) | ((type) << 8) | (nr) | ((size) << 16))
#endif
#ifndef _IO
#define _IO(type, nr)        _IOC(0U, (type), (nr), 0)
#endif
#ifndef _IOR
#define _IOR(type, nr, t)    _IOC(2U, (type), (nr), sizeof(t))
#endif
#ifndef _IOW
#define _IOW(type, nr, t)    _IOC(1U, (type), (nr), sizeof(t))
#endif
#ifndef _IOWR
#define _IOWR(type, nr, t)   _IOC(3U, (type), (nr), sizeof(t))
#endif
#ifndef _IOC_NR
#define _IOC_NR(n)   ((n) & 0xff)
#define _IOC_TYPE(n) (((n) >> 8) & 0xff)
#define _IOC_SIZE(n) (((n) >> 16) & 0x3fff)
#endif