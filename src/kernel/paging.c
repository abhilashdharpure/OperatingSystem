/* paging.c - simple i386 page directory + mapping helpers */
#include "paging.h"
#include <string.h>
#include <stdio.h> /* or your kernel log */
#include <arch/i686/irq.h>
#include <debug.h>
#include "hal/process.h"

#include <stdint.h>
#include <stdbool.h>

// extern uint32_t kernel_phys_offset; // usually 0xC0000000
// uint32_t kernel_phys_offset = 0xC0000000;
uint32_t *kernel_page_directory = 0;
uint32_t kernel_phys_offset = 0xC0000000;   // change if your linker uses another offset


extern uint32_t pmm_alloc_page(void);      /* must be provided by your PMM */
extern void phys_free_page(uint32_t pa);
extern void *phys_to_virt(uint32_t pa);     /* kernel virtual address for physical page */
extern uint32_t virt_to_phys(void *v);      /* optional */
extern void kpanic(const char *msg);        /* optional kernel panic */
extern void kprintf(const char *fmt, ...);  /* or log_info */

void write_cr3(uint32_t pa) {
    __asm__ volatile("mov %0, %%cr3" :: "r"(pa) : "memory");
}

uint32_t virt_to_phys(void *virt)
{
    return (uint32_t)virt - kernel_phys_offset;
}

void *phys_to_virt(uint32_t phys)
{
    return (void *)(phys + kernel_phys_offset);
}


/* Allocate a zero'd physical page and return its PA,
 * or 0 on failure. This is wrapper around pmm_alloc_page.
 */
// static uint32_t alloc_zeroed_page_phys(void) {
//     uint32_t pa = pmm_alloc_page();
//     if (!pa) return 0;
//     void *kv = phys_to_virt(pa);
//     memset(kv, 0, PAGE_SIZE);
//     return pa;
// }

static inline uint32_t alloc_zeroed_page_phys(void) {
    uint32_t pa = pmm_alloc_page();
    if (!pa) return 0;
    memset(phys_to_virt(pa), 0, PAGE_SIZE); // zero the page via kernel VA
    return pa;
}

/* Create a blank page directory (one physical page). Return kernel-virtual pointer
 * to the PD (so caller can write entries). The PD is a physical page, but we return
 * the kernel-virt mapping via phys_to_virt.
 */
uint32_t *create_page_directory(void) {
    uint32_t pd_pa = alloc_zeroed_page_phys();
    if (!pd_pa) return NULL;
    return (uint32_t*)phys_to_virt(pd_pa);
}

/* helper: get PDE pointer (kernel-virt) given pd_virt (kernel-virt pointer to PD) */
static inline uint32_t *pde_for(uint32_t *pd_virt, uint32_t va) {
    (void)pd_virt;
    uint32_t idx = (va >> 22) & 0x3FF;
    return &pd_virt[idx];
}

// ################################################## Option 1 ####################################

// page_dir_t create_user_pd(void)
// {
//     uint32_t pd_pa = pmm_alloc_page();
//     uint32_t *pd_virt = phys_to_virt(pd_pa);
//     memset(pd_virt, 0, PAGE_SIZE);

//     return (page_dir_t){ .pd_phys = pd_pa, .pd_virt = pd_virt };
// }
// uint32_t* pt_from_pde(uint32_t pde)
// {
//     uint32_t pt_pa = pde & 0xFFFFF000;
//     return phys_to_virt(pt_pa);
// }
// int map_page(uint32_t *pd_virt, uintptr_t va, uintptr_t pa, uint32_t flags)
// {
//     uint32_t pd_idx = va >> 22;
//     uint32_t pt_idx = (va >> 12) & 0x3FF;

//     uint32_t pde = pd_virt[pd_idx];
//     uint32_t *pt_virt;

//     if (!(pde & PAGE_PRESENT)) {
//         uint32_t pt_pa = pmm_alloc_page();
//         memset(phys_to_virt(pt_pa), 0, PAGE_SIZE);
//         pd_virt[pd_idx] = pt_pa | PAGE_PRESENT | PAGE_RW | PAGE_USER;
//         pt_virt = phys_to_virt(pt_pa);
//     } else {
//         uint32_t pt_pa = pde & 0xFFFFF000;
//         pt_virt = phys_to_virt(pt_pa);
//     }

//     pt_virt[pt_idx] = (pa & 0xFFFFF000) | (flags & (PAGE_PRESENT | PAGE_RW | PAGE_USER));
//     __asm__ volatile("invlpg (%0)" :: "r"(va) : "memory");
//     return 0;
// }

// ##################################################################################################


// ################################################## Option 2 ####################################


page_dir_t create_user_pd(void)
{
    uint32_t pd_pa = pmm_alloc_page();
    if (!pd_pa) {
        log_info("Paging", "Out of memory creating user page directory");
    }

    // Use identity-mapped virtual for now:
    uint32_t *pd_virt = (uint32_t*)pd_pa; // ONLY if identity-mapped region includes pd_pa
    //uint32_t *pd_virt = phys_to_virt(pd_pa);
    memset(pd_virt, 0, PAGE_SIZE);

    return (page_dir_t){
        .pd_phys = pd_pa,
        .pd_virt = pd_virt
    };
}

uint32_t* pt_from_pde(uint32_t pde)
{
    uint32_t pt_pa = pde & 0xFFFFF000;
    return (uint32_t*)pt_pa; // identity-mapped
}

int map_page(uint32_t *pd, uintptr_t va, uintptr_t pa, uint32_t flags)
{
    uint32_t pd_idx = (va >> 22) & 0x3FF;
    uint32_t pt_idx = (va >> 12) & 0x3FF;

    uint32_t pde = pd[pd_idx];
    uint32_t *pt;

    if (!(pde & PAGE_PRESENT)) {
        uintptr_t pt_pa = pmm_alloc_page();
        if (!pt_pa)
        {
            return -1;
        }

        memset((void*)pt_pa, 0, PAGE_SIZE);  // identity

        pd[pd_idx] = pt_pa | PAGE_PRESENT | PAGE_RW | PAGE_USER;

        pt = (uint32_t*)pt_pa;               // identity
    }
    else
    {
        uintptr_t pt_pa = pde & 0xFFFFF000;
        pt = (uint32_t*)pt_pa;               // identity
    }

    pt[pt_idx] = (pa & 0xFFFFF000) | (flags & (PAGE_PRESENT | PAGE_RW | PAGE_USER));

    __asm__ volatile("invlpg (%0)" :: "r"(va) : "memory");

    return 0;
}

// ##################################################################################################


/* map_region: map [va, va+len) to consecutive physical frames starting at pa_start
 * len must be multiple of PAGE_SIZE (caller responsibility if convenient)
 */
int map_region(uint32_t *pd_phys_ptr, uint32_t va, uint32_t pa_start, uint32_t len, uint32_t flags) {
    if (!pd_phys_ptr) return -1;
    if (va & (PAGE_SIZE - 1)) return -1; /* require page aligned VA */
    uint32_t pages = (len + PAGE_SIZE - 1) / PAGE_SIZE;
    uint32_t cur_va = va;
    uint32_t cur_pa = pa_start;
    for (uint32_t i = 0; i < pages; ++i) {
        if (map_page(pd_phys_ptr, cur_va, cur_pa, flags) != 0) {
            log_info("paging", "map_region: map_page fail at va=0x%08x\n", cur_va);
            return -1;
        }
        cur_va += PAGE_SIZE;
        cur_pa += PAGE_SIZE;
    }
    return 0;
}



/* switch_page_dir: accept pd_virt (kernel-virt pointer to PD page) */
void switch_page_dir(uint32_t *pd_phys_ptr) {
    if (!pd_phys_ptr) return;
    /* get physical address: caller gave kernel-virt pointing into phys page; convert back */
    uint32_t pd_pa = 0;
#ifdef PHYS_FROM_VIRT /* if you have virt_to_phys, prefer that */
    pd_pa = virt_to_phys(pd_phys_ptr);
#else
    /* assume identity mapping (kernel-virtual == physical) */
    pd_pa = (uint32_t)pd_phys_ptr;
#endif
    write_cr3(pd_pa);
}

void clone_kernel_mappings(uint32_t *user_pd)
{
    // kernel usually mapped in high 1GB: 0xC0000000+
    for (uint32_t i = 768; i < 1024; i++) {
        user_pd[i] = kernel_page_directory[i];
    }
}

extern uint32_t kernel_phys_offset; /* 0xC0000000 typical */

#define SERIAL_PORT_COM1 0x3F8



// /* wait until transmitter holding register empty (bit 5) */
// void early_serial_wait_tx_ready(void) {
//     while (!(i686_inb(SERIAL_PORT_COM1 + 5) & 0x20)) { /* spin */ }
// }

// void early_serial_putc(char c) {
//     early_serial_wait_tx_ready();
//     i686_outb(SERIAL_PORT_COM1 + 0, (uint8_t)c);
// }

// /* print 8-bit nibble as hex char */
// void _put_hex_nibble(uint8_t n) {
//     char c = (n < 10) ? ('0' + n) : ('A' + (n - 10));
//     early_serial_putc(c);
// }

// /* print 32-bit hex, with 0x prefix */
// void early_serial_print_hex32(uint32_t v) {
//     early_serial_write("0x");
//     for (int i = 7; i >= 0; --i) {
//         _put_hex_nibble((v >> (i*4)) & 0xF);
//     }
// }


// void early_serial_write(const char *s) {
//     while (*s) {
//         if (*s == '\n') early_serial_putc('\r'); // optional CR
//         early_serial_putc(*s++);
//     }
// }
/* init COM1 minimal (do once) */
static inline void early_serial_init(void) {
    // i686_outb(SERIAL_PORT_COM1 + 1, 0x00); // disable all interrupts
    // i686_outb(SERIAL_PORT_COM1 + 3, 0x80); // enable DLAB (set baud rate divisor)
    // i686_outb(SERIAL_PORT_COM1 + 0, 0x03); // divisor low byte (38400 baud if 115200 base -> 0x03)
    // i686_outb(SERIAL_PORT_COM1 + 1, 0x00); // divisor high byte
    // i686_outb(SERIAL_PORT_COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
    // i686_outb(SERIAL_PORT_COM1 + 2, 0xC7); // enable FIFO, clear them, with 14-byte threshold
    // i686_outb(SERIAL_PORT_COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

void set_kernel_page_directory(void)
{
    uint32_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    uint32_t pd_pa = cr3 & 0xFFFFF000U;
    /* kernel_phys_offset should be the virtual base where physical memory is mapped
       e.g. 0xC0000000 for a high-half kernel. */
    kernel_page_directory = (uint32_t*)(pd_pa + kernel_phys_offset);
    /* Do NOT call any log helper here that may touch page tables or allocators.
       If you want to print, use the early-serial routine (see below). */

    //    /* after computing pd_pa above */
    // early_serial_init();
    // early_serial_write("CR3 PA=");
    // early_serial_print_hex32(pd_pa);
    // early_serial_write("\nKPD virt=");
    // early_serial_print_hex32((uint32_t)kernel_page_directory);
    // early_serial_write("\n");

}



// /* early_serial.c — tiny, self-contained serial writer (safe early) */

// #include <stdint.h>

// /* port IO: provide your existing inb/i686_outb if available; else add these */
// static inline void i686_outb(uint16_t port, uint8_t val) {
//     __asm__ volatile ("i686_outb %0, %1" : : "a"(val), "Nd"(port));
// }
// static inline uint8_t inb(uint16_t port) {
//     uint8_t v;
//     __asm__ volatile ("inb %1, %0" : "=a"(v) : "Nd"(port));
//     return v;
// }



// /* tiny integer print (dec) for small numbers, optional */
// static void early_serial_print_u32(uint32_t v) {
//     char buf[16];
//     int pos = 0;
//     if (v == 0) { early_serial_putc('0'); return; }
//     while (v && pos < (int)sizeof(buf)-1) { buf[pos++] = '0' + (v % 10); v /= 10; }
//     while (pos) early_serial_putc(buf[--pos]);
// }

// /* test function you can call directly */
// void early_serial_test_print(void) {
//     early_serial_init();
//     early_serial_write("SERIAL READY\n");
//     early_serial_write("Hello from early serial\n");
//     early_serial_print_hex32(0x1234ABCD);
//     early_serial_write("\n");
// }


// /* early_dump.c — uses early_serial */

// extern void early_serial_init(void);
// extern void early_serial_write(const char *);
// extern void early_serial_print_hex32(uint32_t);
// extern void early_serial_print_u32(uint32_t);



// /* call from the place that earlier did log_info */
// void early_dump_kernel_pd(uint32_t *kernel_pd, uint32_t *user_pd) {
//     early_serial_init();
//     early_serial_write("EARLY DUMP: kernel_page_directory = ");
//     early_serial_print_hex32((uint32_t)kernel_pd);
//     early_serial_write("\n");

//     early_serial_write("EARLY DUMP: user_pd = ");
//     early_serial_print_hex32((uint32_t)user_pd);
//     early_serial_write("\n");

//     /* Dump a few PDEs: 0, 1, 2, 768..772 */
//     for (int i = 0; i < 4; ++i) {
//         early_serial_write("PDE[");
//         early_serial_print_u32(i);
//         early_serial_write("] = ");
//         early_serial_print_hex32(user_pd[i]);
//         early_serial_write("\n");
//     }
//     for (int i = 768; i < 772; ++i) {
//         early_serial_write("PDE[");
//         early_serial_print_u32(i);
//         early_serial_write("] = ");
//         early_serial_print_hex32(user_pd[i]);
//         early_serial_write("\n");
//     }
// }


// void enter_user_mode_from_process(Process *p)
// {
//     if (!p || !p->page_directory) {
//         log_error("EXEC", "enter_user_mode: invalid process or missing page_directory");
//         return;
//     }

//     uint32_t pd_phys = virt_to_phys(p->page_directory);
//     if (!pd_phys) {
//         log_error("EXEC", "enter_user_mode: virt_to_phys failed");
//         return;
//     }

//     log_info("EXEC", "enter_user_mode_from_process write_cr3");

//     // early_serial_test_print();

//     // early_dump_kernel_pd(kernel_page_directory, p->page_directory);

//     // /* DEBUG: dump kernel_page_directory and new pd */
//     // log_info("EXEC", "kernel_page_directory = %p", (void*)kernel_page_directory);
//     // log_info("EXEC", "kernel_page_directory = 0x%x", (uint32_t)kernel_page_directory);

//     // dump_pd_info(kernel_page_directory);
//     // dump_pd_info(p->page_directory);

//     /* Load the process page directory (CR3) */
//     write_cr3(pd_phys);

//     log_info("EXEC", "enter_user_mode_from_process After write_cr3");


//     /* Load user data selectors into DS/ES/FS/GS while still in ring0.
//        Loading SS must be done by IRET, so we push SS/ESP in the iret frame below. */
//     __asm__ volatile (
//         "movw %[udsel], %%ax\n\t"
//         "movw %%ax, %%ds\n\t"
//         "movw %%ax, %%es\n\t"
//         "movw %%ax, %%fs\n\t"
//         "movw %%ax, %%gs\n\t"
//         : /* no outputs */
//         : [udsel] "i" (USER_DS)
//         : "ax", "memory"
//     );

//     log_info("EXEC", "enter_user_mode_from_process switch to user mode............");


//     /* Now switch to user mode: build an iret frame and iret.
//        Interrupts should be disabled while manipulating the stack frame */
//     __asm__ volatile (
//         "cli\n\t"                         /* disable interrupts during the switch */
//         "pushl %[udsel]\n\t"             /* SS (user data selector) */
//         "pushl %[esp]\n\t"               /* ESP (user stack top) */
//         "pushf\n\t"                      /* EFLAGS */
//         "pushl %[ucsel]\n\t"             /* CS (user code selector) */
//         "pushl %[eip]\n\t"               /* EIP (entry point) */
//         "iret\n\t"
//         :
//         : [udsel] "r" ((uint32_t)USER_DS),
//           [esp]   "r" ((uint32_t)p->regs.esp),
//           [ucsel] "r" ((uint32_t)USER_CS),
//           [eip]   "r" ((uint32_t)p->regs.eip)
//         : "memory"
//     );

//     /* we should never reach here in the kernel if iret succeeded */
//     log_critical("EXEC", "enter_user_mode: iret returned unexpectedly");
//     for(;;);
// }

void enter_user_mode_from_process(Process *p)
{
    if (!p || !p->page_directory) {
        log_error("EXEC", "enter_user_mode: invalid process or missing page_directory");
        return;

    }

    // ################################################## Option 1 ####################################
    // uint32_t pd_phys = virt_to_phys(p->page_directory);
    // ##################################################################################################


    // ################################################## Option 1 ####################################
    uint32_t pd_phys = p->cr3;
    // ##################################################################################################


    if (!pd_phys) {
        log_error("EXEC", "enter_user_mode: virt_to_phys failed");
        return;
    }

    log_info("EXEC", "enter_user_mode_from_process write_cr3");

    /* Switch to process page directory */
    write_cr3(pd_phys);

    log_info("EXEC", "enter_user_mode_from_process After write_cr3");

    /*
     * Load user data selectors while still in ring 0.
     * SS MUST NOT be loaded here — it must be loaded by IRET.
     */
    __asm__ volatile (
        "movw %[udsel], %%ax\n\t"
        "movw %%ax, %%ds\n\t"
        "movw %%ax, %%es\n\t"
        "movw %%ax, %%fs\n\t"
        "movw %%ax, %%gs\n\t"
        :
        : [udsel] "i" (USER_DS)
        : "ax", "memory"
    );

    log_info("EXEC", "enter_user_mode_from_process switching to user mode");

    /*
     * Build a *safe* EFLAGS value for ring 3.
     * We MUST NOT reuse kernel EFLAGS directly.
     */
    uint32_t user_eflags;
    __asm__ volatile (
        "pushf\n\t"
        "pop %0\n\t"
        : "=r"(user_eflags)
    );

    /* Enable interrupts in user mode */
    user_eflags |= (1 << 9);        /* IF = 1 */

    /* Clear IOPL (must be 0 for ring 3) */
    user_eflags &= ~(3 << 12);      /* IOPL = 0 */

    /*
     * Now switch to user mode using IRET.
     * Stack frame layout (top → bottom):
     *   SS
     *   ESP
     *   EFLAGS
     *   CS
     *   EIP
     */
    __asm__ volatile (
        "cli\n\t"                         /* no interrupts during transition */
        "pushl %[udsel]\n\t"             /* SS */
        "pushl %[esp]\n\t"               /* ESP */
        "pushl %[eflags]\n\t"            /* sanitized EFLAGS */
        "pushl %[ucsel]\n\t"             /* CS */
        "pushl %[eip]\n\t"               /* EIP */
        "iret\n\t"
        :
        : [udsel]  "r" ((uint32_t)USER_DS),
          [esp]    "r" ((uint32_t)p->regs.esp),
          [eflags] "r" (user_eflags),
          [ucsel]  "r" ((uint32_t)USER_CS),
          [eip]    "r" ((uint32_t)p->regs.eip)
        : "memory"
    );

    /* We should NEVER reach here */
    log_critical("EXEC", "enter_user_mode: iret returned unexpectedly");
    for (;;);
}
