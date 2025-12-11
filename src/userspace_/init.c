// /bin/init.c
// Minimal user-space init for testing

typedef unsigned int size_t;

#define SYS_WRITE 1
#define STDOUT 1

// syscall wrapper
static inline int sys_write(int fd, const void *buf, size_t count) {
    int ret;
    __asm__ volatile (
        "int $0x80"        // trigger syscall interrupt
        : "=a"(ret)        // output: eax = return value
        : "a"(SYS_WRITE),  // input: eax = syscall number
          "b"(fd),         // ebx = fd
          "c"(buf),        // ecx = buffer
          "d"(count)       // edx = length
        : "memory"
    );
    return ret;
}

void main() {
    const char *msg = "[USERSPACE] /bin/init started!\n";
    sys_write(STDOUT, msg, 30);

    // Loop indefinitely
    while (1) { }
}







// #include <stdint.h>
// #define PORT 0x3F8  // COM1

// static inline uint8_t inb(uint16_t port) {
//     uint8_t ret;
//     __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
//     return ret;
// }

// static inline void outb(uint16_t port, uint8_t val) {
//     __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
// }

// static void debug_putchar(char c) {
//     while (!(inb(PORT+5) & 0x20));
//     outb(PORT, c);
// }


// int write(int fd, const void *buf, uint32_t len);

// void _start(void)
// {
//     // debug_putchar('U');  // mark that _start is reached
//     // const char msg[] = "Hello from user space!\n";

//     // write(1, msg, sizeof(msg) - 1);

//     write(1, "Hello from user space!\n", 23);

//     for (;;);
// }
