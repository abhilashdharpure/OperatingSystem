[BITS 32]
SECTION .boot32
global multiboot_entry

extern pml4_table_boot
extern gdt64_desc_boot
extern kernel_low_entry

multiboot_entry:
    cli

    ; Enable PAE
    mov eax, cr4
    or  eax, 1 << 5
    mov cr4, eax

    ; Load PML4 physical address (identity-mapped)
    mov eax, pml4_table_boot      ; e.g., 0x00101000
    mov cr3, eax

    ; Enable long mode in IA32_EFER
    mov ecx, 0xC0000080
    rdmsr
    or  eax, 1 << 8               ; LME
    wrmsr

    ; Enable paging
    mov eax, cr0
    or  eax, 1 << 31              ; PG
    mov cr0, eax

    ; Load 64-bit GDT (must be identity-mapped too)
    lgdt [gdt64_desc_boot]

    ; Pass multiboot info in RDI (lower 32 bits in EDI)
    mov edi, ebx

    ; Far jump to 64-bit stub, still in low identity region
    jmp 0x08:kernel_low_entry
