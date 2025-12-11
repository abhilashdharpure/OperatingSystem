// /bin/init.c
// Minimal user-space init for testing
#include <stdint.h>
typedef unsigned int size_t;

#define SYS_WRITE 1
#define STDOUT 1

// syscall numbers
#define SYSCALL_WRITE_CHAR 0

static inline void syscall_write(const char *buf, uint32_t len)
{
    asm volatile (
        "movl $1, %%eax\n"     // SYS_WRITE
        "movl $1, %%ebx\n"     // STDOUT
        "movl %0, %%ecx\n"     // buffer
        "movl %1, %%edx\n"     // length
        "int $0x80"
        :
        : "r"(buf), "r"(len)
        : "eax","ebx","ecx","edx"
    );
}


const char msg[] = "Hello from user space!\n";

void _start() {
    syscall_write(msg, sizeof(msg)-1);
    while(1);
}

// static inline void syscall_write_char(char c) {
//     asm volatile (
//         "movl $0, %%eax\n"        // syscall number 0 = write_char
//         "movb %0, %%bl\n"         // character to print in BL
//         "int $0x80"               // software interrupt
//         :
//         : "q"(c)                  // 'q' tells GCC to use 8-bit reg (al/bl/cl/dl)
//         : "eax", "ebx"
//     );
// }


// int main() {
//     // Your user-space log test
//     const char *msg = "Hello from user space!\n";
//     for (const char *p = msg; *p; ++p) {
//         // simple syscall to write char (replace with your kernel write method)
//         syscall_write_char(*p);
//     }
//     return 0;
// }

// // src/main.c
// void _start() {
//     main();       // call your main()
//     while(1);     // hang after main returns
// }
