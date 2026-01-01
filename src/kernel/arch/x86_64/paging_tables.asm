[BITS 64]
SECTION .data.boot
align 4096

%define PML4_FLAGS   0x003          ; Present | RW
%define PDPTE_1G     0x083          ; Present | RW | PS (1 GiB page)

global pml4_table_boot

; PML4: only entry 0 used
pml4_table_boot:
    dq pdpt_identity + PML4_FLAGS   ; PML4[0]
    times 511 dq 0

align 4096

; PDPT: single 1GiB page mapping phys 0..1GiB at VA 0..1GiB
pdpt_identity:
    dq 0x0000000000000000 + PDPTE_1G
    times 511 dq 0
