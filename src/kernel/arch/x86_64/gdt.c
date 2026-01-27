// gdt.c
#include "gdt.h"
#include "debug.h"

extern void gdt_flush(uint64_t gdtr);

GDTEntry g_GDT[7];
GDTR g_GDT_Ptr;

static inline GDTEntry gdt_make_entry(uint8_t access, uint8_t flags)
{
    GDTEntry e = {0};
    e.limit_low = 0;
    e.base_low  = 0;
    e.base_mid  = 0;
    e.access    = access;
    e.gran      = (flags & 0xF0);   // limit_high = 0, flags in high nibble
    e.base_high = 0;
    return e;
}

static void dump_descriptor(const char *name, uint16_t sel)
{
    uint16_t index = sel >> 3;
    GDTEntry *e = &g_GDT[index];
    uint8_t access = e->access;
    uint8_t flags  = e->gran >> 4;

    log_info("GDT", "%s sel=0x%x access=0x%02x flags=0x%02x",
             name, sel, access, flags);
}


void gdt_init(void)
{
    g_GDT[0] = (GDTEntry){0};

    // kernel code: present | ring0 | code | RW, L=1, G=1
    g_GDT[1] = gdt_make_entry(
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE | GDT_ACCESS_RW,
        GDT_FLAG_LONG_MODE | GDT_FLAG_GRAN_4K
    );

    // kernel data: present | ring0 | data | RW
    g_GDT[2] = gdt_make_entry(
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA | GDT_ACCESS_RW,
        0
    );

    // user code: present | ring3 | code | RW, L=1, G=1
    g_GDT[3] = gdt_make_entry(
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE | GDT_ACCESS_RW,
        GDT_FLAG_LONG_MODE | GDT_FLAG_GRAN_4K
    );

    // user data: present | ring3 | data | RW
    g_GDT[4] = gdt_make_entry(
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA | GDT_ACCESS_RW,
        0
    );

    g_GDT[5] = (GDTEntry){0};
    g_GDT[6] = (GDTEntry){0};

    g_GDT_Ptr.limit = sizeof(g_GDT) - 1;
    g_GDT_Ptr.base  = (uint64_t)&g_GDT;

    gdt_flush((uint64_t)&g_GDT_Ptr);

    __asm__ volatile (
        "mov %0, %%ds\n"
        "mov %0, %%es\n"
        "mov %0, %%ss\n"
        :
        : "r"(KERNEL_DATA_SELECTOR)
        : "memory"
    );

    dump_descriptor("USER_CODE", USER_CODE_SELECTOR);

}
