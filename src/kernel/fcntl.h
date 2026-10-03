#pragma once

#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_CREAT    0000100   /* 0x40    */
#define O_EXCL     0000200   /* 0x80    */
#define O_TRUNC    0001000   /* 0x200   */
#define O_APPEND   0002000
#define O_NONBLOCK 0004000   /* 0x800   */
#define O_CLOEXEC  02000000  /* 0x80000 */

#define F_DUPFD             0
#define F_GETFD             1
#define F_SETFD             2
#define F_GETFL             3
#define F_SETFL             4       
#define F_DUPFD_CLOEXEC     1030   // Linux x86_64 ABI


