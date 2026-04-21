[BITS 64]
SECTION .data.boot
align 4096

%define PAGE_PRESENT  0x001
%define PAGE_RW       0x002
%define PAGE_PS       0x080

%define PML4_FLAGS    (PAGE_PRESENT | PAGE_RW)

global pml4_table
global pml4_table_boot

pml4_table:
pml4_table_boot:
    dq pdpt_identity + PML4_FLAGS
    times 511 dq 0

align 4096
pdpt_identity:
    dq pd_identity + PML4_FLAGS
    times 511 dq 0

align 4096
pd_identity:
%assign i 0
%rep 512
    dq (i * 0x200000) | PAGE_PRESENT | PAGE_RW | PAGE_PS
%assign i i+1
%endrep
