[BITS 32]
SECTION .boot32
global multiboot_entry

extern pml4_table_boot
extern gdt64_desc_boot
extern kernel_low_entry

multiboot_entry:
    cli

    mov dx, 0x3F8
    mov al, '1'
    out dx, al

    ; Enable PAE
    mov eax, cr4
    or  eax, 1 << 5
    mov cr4, eax

    mov dx, 0x3F8
    mov al, '2'
    out dx, al

    ; Load PML4 physical address (identity-mapped)
    mov eax, pml4_table_boot      ; e.g., 0x00101000
    mov cr3, eax

    mov dx, 0x3F8
    mov al, '3'
    out dx, al

    ; Enable long mode in IA32_EFER
    mov ecx, 0xC0000080
    rdmsr
    or  eax, 1 << 8               ; LME
    wrmsr

    mov dx, 0x3F8
    mov al, '4'
    out dx, al

    ; Enable paging
    mov eax, cr0
    or  eax, 1 << 31              ; PG
    mov cr0, eax

    mov dx, 0x3F8
    mov al, '5'
    out dx, al

    ; Load 64-bit GDT (must be identity-mapped too)
    lgdt [gdt64_desc_boot]

    mov dx, 0x3F8
    mov al, '6'
    out dx, al

    ; Pass multiboot info in RDI (lower 32 bits in EDI)
    mov edi, ebx

    mov dx, 0x3F8
    mov al, '7'
    out dx, al

    ; Far jump to 64-bit stub, still in low identity region
    jmp 0x08:kernel_low_entry
