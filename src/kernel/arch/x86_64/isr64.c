// isr64.c
#include "isr64.h"
#include "debug.h"
#include "hal/process.h"
#include "arch/x86_64/serial.h"
#include "arch/x86_64/irq.h"
#include "paging.h"

ISR64Handler g_ISR64Handlers[256];
extern Process *current_process;

void x64_ISR_InitializeGates();

void x64_ISR_Initialize()
{
    x64_ISR_InitializeGates();
    log_info("Main", "x64_ISR_InitializeGates");

    // for (int i = 0; i < 256; i++)
    //     x64_IDT_EnableGate(i);

    for (int i = 0; i < 32; i++)
    {
        x64_IDT_EnableGate(i);
    }

    // for (int i = 32; i < 48; i++)
    // {
    //     x64_IDT_EnableGate(i);
    // }

    // Hardware IRQs (0x20–0x2F)
    for (int i = PIC_REMAP_OFFSET; i < PIC_REMAP_OFFSET + 16; i++)
    {
        x64_IDT_EnableGate(i);
    }
    
    x64_IDT_EnableGate(0x80);
}

static inline uint64_t read_cr2(void)
{
    uint64_t v; 
    __asm__ volatile("mov %%cr2, %0" : "=r"(v));
    return v;
}



void dump_iret_frame(uint64_t *sp)
{
    log_info("IRET", "RIP=%lx CS=%lx RFLAGS=%lx RSP=%lx SS=%lx",
        sp[0], sp[1], sp[2], sp[3], sp[4]);
}

void x64_ISR_Handler(ISRFrame64* r)
{
    uint8_t vec = (uint8_t)r->vector; 
    if (vec == 1)
    {
        log_warning("EXC", "#DB ignored (TF was set)");
        return;
    }

    // Temporary skiping PIT
    if (vec!= 32)
    {
        //log_error("ISR64", "x64_ISR_Handler Interupt r->vector = %u", vec);
    }

    if (vec < 32)
    {
        if (vec == 6)
        {
            log_info("EXC", "UD at rip=0x%llx", (unsigned long long)r->cpu.rip);
            debug_dump_va_mapping(current_process->page_directory, r->cpu.rip);
            // debug_dump_user_bytes(current_process, r->cpu.rip, 0x40);
        }
        else if (vec == 7)
        {
            uint64_t cr0_before, cr0_after;

            __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0_before));
            uint64_t rip = r->cpu.rip;
            uint64_t cs  = r->cpu.cs;

            log_critical("NM",
                "#NM at rip=0x%llx cs=0x%llx CR0=0x%llx",
                (unsigned long long)rip,
                (unsigned long long)cs,
                (unsigned long long)cr0_before);

            // optional: dump mapping + bytes at RIP
            debug_dump_va_mapping(current_process->page_directory, rip);
            debug_dump_user_bytes(current_process, rip, 0x40);

            // keep your CR0 fixup (even though EM/TS are 0)
            uint64_t cr0 = cr0_before;
            cr0 &= ~(1ULL << 3); // TS = 0
            cr0 &= ~(1ULL << 2); // EM = 0
            __asm__ volatile ("mov %0, %%cr0" :: "r"(cr0) : "memory");
            __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0_after));
            log_info("NM", "CR0 after =0x%llx", (unsigned long long)cr0_after);

            return;
        }
        else if (vec == 13)
        {
            // General Protection Fault
            log_critical("GP",
                "#GP: error=%llx rip=%p cs=%llx ss=%llx rflags=%llx",
                r->error,
                (void*)r->cpu.rip,
                r->cpu.cs,
                r->cpu.ss,
                r->cpu.rflags
            );

            // Read CR3
            uint64_t cr3;
            __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
            log_critical("GP", "CR3=0x%llx", (unsigned long long)cr3);

            // Dump general-purpose registers (top-level fields in ISRFrame64)
            log_critical("GPREG", "RAX=0x%llx RBX=0x%llx RCX=0x%llx RDX=0x%llx",
             (unsigned long long)r->rax, (unsigned long long)r->rbx,
             (unsigned long long)r->rcx, (unsigned long long)r->rdx);

            log_critical("GPREG",
                "RSI=0x%llx RDI=0x%llx RBP=0x%llx R8 =0x%llx",
                (unsigned long long)r->rsi, (unsigned long long)r->rdi, (unsigned long long)r->rbp, (unsigned long long)r->r8);
            log_critical("GPREG",
                "R9 =0x%llx R10=0x%llx R11=0x%llx R12=0x%llx",
                (unsigned long long)r->r9, (unsigned long long)r->r10, (unsigned long long)r->r11, (unsigned long long)r->r12);
            log_critical("GPREG",
                "R13=0x%llx R14=0x%llx R15=0x%llx",
                (unsigned long long)r->r13, (unsigned long long)r->r14, (unsigned long long)r->r15);

            if (current_process && current_process->page_directory) {
                // Mapping info for RIP and RSP (RIP/RSP are in r->cpu)
                uint64_t rip_va = r->cpu.rip;
                uint64_t rsp_va = r->cpu.rsp;
                uint64_t rip_pa = get_mapped_phys(current_process->page_directory, rip_va);
                uint64_t rsp_pa = get_mapped_phys(current_process->page_directory, rsp_va);

                log_critical("GP", "RIP VA=0x%llx -> PA=0x%llx", (unsigned long long)rip_va, rip_pa);
                log_critical("GP", "RSP VA=0x%llx -> PA=0x%llx", (unsigned long long)rsp_va, rsp_pa);

                // Dump PTE chain for both addresses
                debug_dump_va_mapping(current_process->page_directory, rip_va);
                debug_dump_va_mapping(current_process->page_directory, rsp_va);

                // Dump page contents around RIP and RSP (page-aligned)
                // debug_dump_user_bytes(current_process, (rip_va & ~(PAGE_SIZE - 1)), 256);
                // debug_dump_user_bytes(current_process, (rsp_va & ~(PAGE_SIZE - 1)), 256);

                // Dump first 32 qwords from user stack (starting at RSP)
                for (int i = 0; i < 32; ++i) {
                    uint64_t va = rsp_va + i * 8;
                    uint64_t pa = get_mapped_phys(current_process->page_directory, va);
                    uint64_t val = 0;
                    if (pa) {
                        uint8_t *k = (uint8_t *)phys_to_virt(pa & ~(PAGE_SIZE - 1));
                        val = *(uint64_t *)(k + (va & (PAGE_SIZE - 1)));
                    }
                    log_critical("GPSTACK", " [0x%llx] -> PA=0x%llx : 0x%016llx", va, pa, val);
                }

                // Best-effort user backtrace via saved RBP chain (saved RBP is top-level r->rbp)
                uint64_t user_rbp = r->rbp;
                log_critical("GPBT", "Attempting user backtrace from RBP=0x%llx", user_rbp);
                for (int frame = 0; frame < 16 && user_rbp; ++frame) {
                    uint64_t rbp_page_pa = get_mapped_phys(current_process->page_directory, user_rbp & ~(PAGE_SIZE - 1));
                    if (!rbp_page_pa) {
                        log_critical("GPBT", " frame %d: RBP 0x%llx -> unmapped", frame, user_rbp);
                        break;
                    }
                    uint8_t *rbp_k = (uint8_t *)phys_to_virt(rbp_page_pa & ~(PAGE_SIZE - 1));
                    uint64_t saved_rbp = *(uint64_t *)(rbp_k + (user_rbp & (PAGE_SIZE - 1)));
                    uint64_t ret_addr  = *(uint64_t *)(rbp_k + (user_rbp & (PAGE_SIZE - 1)) + 8);
                    log_critical("GPBT", " frame %d: RBP=0x%llx saved_rbp=0x%llx ret=0x%llx",
                                frame, user_rbp, saved_rbp, ret_addr);
                    if (ret_addr == 0) break;
                    user_rbp = saved_rbp;
                }
            }

            // Dump the IRET frame (immediately after ISRFrame64)
            uint64_t *iret = (uint64_t *)((uint8_t *)r + sizeof(ISRFrame64));
            log_critical("GP", "IRET frame: RIP=0x%llx CS=0x%llx RFLAGS=0x%llx RSP=0x%llx SS=0x%llx",
                        (unsigned long long)iret[0],
                        (unsigned long long)iret[1],
                        (unsigned long long)iret[2],
                        (unsigned long long)iret[3],
                        (unsigned long long)iret[4]);


            dump_iret_frame(iret);

            // Panic to preserve logs
            panic();
        }


        else if (vec == 14) {
            uint64_t cr2 = read_cr2();
            uint64_t err = r->error;

            log_critical("PF", "Page fault: cr2=%p error=%llx", (void*)cr2, err);

            // user vs kernel
            bool from_user = (r->cpu.cs & 3) == 3;

            if (from_user && current_process) {
                Process *p = current_process;

                if (current_process) {
                    debug_check_va(current_process, cr2);
                    debug_dump_user_bytes(current_process, 0x7ffffe88 & ~(PAGE_SIZE-1), 256);
                }

                // Stack growth condition: fault below current stack_base,
                // above guard, and above soft limit.
                if (cr2 < p->stack_base &&
                    cr2 >= p->stack_soft_limit_bottom &&
                    cr2 >= p->stack_guard_page &&
                    cr2 < USER_STACK_TOP) {

                    uint64_t va_page = cr2 & ~(PAGE_SIZE - 1);
                    const uint64_t flags = PAGE_PRESENT | PAGE_RW | PAGE_USER | PAGE_NX;

                    // Grow down one page at a time until we cover cr2
                    while (va_page < p->stack_base) {
                        uint64_t pa = pmm_alloc_page();
                        if (!pa) {
                            log_critical("PF", "stack growth: out of memory");
                            break;
                        }
                        memset(phys_to_virt(pa), 0, PAGE_SIZE);
                        if (map_page(p->page_directory, va_page, pa, flags) != 0) {
                            log_critical("PF", "stack growth: map_page failed");
                            break;
                        }
                        p->stack_base = va_page;
                        va_page      += PAGE_SIZE;
                    }

                    log_info("PF", "stack grown: new stack_base=0x%llx", p->stack_base);
                    return; // resume user
                }

                // Guard page hit or below soft limit → treat as stack overflow
                if (cr2 < p->stack_guard_page || cr2 < p->stack_soft_limit_bottom) {
                    log_critical("PF", "stack overflow: cr2=0x%llx", cr2);
                    // TODO: send SIGSEGV / kill process
                    panic();
                }
            }

            // existing debug + panic for now
            debug_dump_va_mapping(current_process->page_directory, r->cpu.rip);
            log_critical("EXC",
                "vec=%llu rip=%p cs=%llx rflags=%llx",
                vec,
                (void*)r->cpu.rip,
                r->cpu.cs,
                r->cpu.rflags
            );
            panic();
        }



        log_critical("EXC",
            "vec=%llu rip=%p cs=%llx rflags=%llx",
            vec,
            (void*)r->cpu.rip,
            r->cpu.cs,
            r->cpu.rflags
        );

        panic();
    }

    if (r->vector == 0x80) {
        x64_syscall_handler(r);
        return;
    }

    if (g_ISR64Handlers[vec]) {
        g_ISR64Handlers[vec](r);
    }
}

void x64_ISR_RegisterHandler(int interrupt, ISR64Handler handler)
{
    g_ISR64Handlers[interrupt] = handler;
    x64_IDT_EnableGate(interrupt);
}
