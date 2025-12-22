[BITS 32]
section .boot32

global multiboot_entry
extern pml4_table
extern kernel_entry

multiboot_entry:
    cli

    ; 1
    mov dx, 0x3F8
    mov al, '1'
    out dx, al

    ; load our own unified GDT (legacy + long mode)
    lgdt [gdt_ptr]

    ; load data segments from our GDT
    mov ax, 0x10           ; data selector
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; 2
    mov dx, 0x3F8
    mov al, '2'
    out dx, al

    ; enable PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; 3
    mov dx, 0x3F8
    mov al, '3'
    out dx, al

    ; enable LME
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1
    wrmsr

    ; 4
    mov dx, 0x3F8
    mov al, '4'
    out dx, al

    ; load PML4 (identity-mapped)
    mov eax, pml4_table
    mov cr3, eax

    ; 5
    mov dx, 0x3F8
    mov al, '5'
    out dx, al

    ; enable paging
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; 6
    mov dx, 0x3F8
    mov al, '6'
    out dx, al

    ; 7
    mov dx, 0x3F8
    mov al, '7'
    out dx, al

    ; far jump into 64-bit code segment (0x08 in our GDT)
    ; jmp 0x08:long_mode_entry
    jmp long_mode_entry


; unified GDT (correct long-mode descriptors)
align 8
gdt:
    dq 0                        ; null
    dq 0x00209A0000000000       ; 0x08: long-mode code (L=1, D=0)
    dq 0x0000920000000000       ; 0x10: data          (L=0, D=0)

gdt_end:

align 8
gdt_ptr:
    dw gdt_end - gdt - 1
    dd gdt


[BITS 64]
long_mode_entry:
    mov dx, 0x3F8
    mov al, 'L'
    out dx, al

    jmp kernel_entry


; [BITS 32]
; section .text.boot32  

; global multiboot_entry
; extern kernel_entry
; extern pml4_table

; multiboot_entry:
;     cli

;     ; --- serial: '1' ---
;     mov dx, 0x3F8
;     mov al, '1'
;     out dx, al

;     ; load *single* GDT which we'll use for both modes
;     lgdt [gdt_ptr]

;     ; load data segments (will use same data descriptor in 32-bit & 64-bit)
;     mov ax, 0x10           ; data selector
;     mov ds, ax
;     mov es, ax
;     mov ss, ax

;     ; --- serial: '2' ---
;     mov dx, 0x3F8
;     mov al, '2'
;     out dx, al

;     ; --- enable PAE ---
;     mov eax, cr4
;     or eax, 1 << 5         ; PAE
;     mov cr4, eax

;     ; --- serial: '3' ---
;     mov dx, 0x3F8
;     mov al, '3'
;     out dx, al

;     ; --- enable long mode (LME) in EFER ---
;     mov ecx, 0xC0000080    ; IA32_EFER
;     rdmsr
;     or eax, 1              ; set LME
;     wrmsr

;     ; --- serial: '4' ---
;     mov dx, 0x3F8
;     mov al, '4'
;     out dx, al

;     ; --- load PML4 into CR3 ---
;     mov eax, pml4_table
;     mov cr3, eax

;     ; --- serial: '5' ---
;     mov dx, 0x3F8
;     mov al, '5'
;     out dx, al

;     ; --- enable paging (PG=1) ---
;     mov eax, cr0
;     or eax, 0x80000000     ; PG (PE already set by GRUB)
;     mov cr0, eax

;     ; --- serial: '6' ---
;     mov dx, 0x3F8
;     mov al, '6'
;     out dx, al

;     ; now LME+PG are set and the GDT already has 64-bit code at 0x08
;     ; we just far jump into it
;     ; --- serial: '7' ---
;     mov dx, 0x3F8
;     mov al, '7'
;     out dx, al

;     jmp 0x08:long_mode_entry


; ; -----------------------------
; ; Unified GDT (legacy + long mode)
; ; -----------------------------
; align 8
; gdt:
;     dq 0                        ; null

;     ; 0x08: 64-bit code segment: base=0, limit=4GB, G=1, L=1, D=0
;     dq 0x00AF9A000000FFFF

;     ; 0x10: data segment: base=0, limit=4GB, G=1, L=0, D=1
;     dq 0x00AF92000000FFFF

; gdt_end:

; align 8
; gdt_ptr:
;     dw gdt_end - gdt - 1
;     dd gdt                     ; 32-bit base


; [BITS 64]
; long_mode_entry:
;     mov dx, 0x3F8
;     mov al, 'L'
;     out dx, al

;     jmp kernel_entry

; BITS 32
; global multiboot_entry
; extern kernel_entry
; extern pml4_table

; multiboot_entry:
;     cli

;     ; --- serial: '1' ---
;     mov dx, 0x3F8
;     mov al, '1'
;     out dx, al

;     ; --- load 64-bit GDT ---
;     ; lgdt [gdt64_ptr]
;     lgdt [gdt32_ptr]

;     ; load data segments from our GDT
;     mov ax, 0x10           ; data selector
;     mov ds, ax
;     mov es, ax
;     mov ss, ax

;     ; --- serial: '2' ---
;     mov dx, 0x3F8
;     mov al, '2'
;     out dx, al

;     ; --- enable PAE ---
;     mov eax, cr4
;     or eax, 1 << 5         ; PAE
;     mov cr4, eax

;     ; --- serial: '3' ---
;     mov dx, 0x3F8
;     mov al, '3'
;     out dx, al

;     ; --- enable long mode (LME) in EFER ---
;     mov ecx, 0xC0000080    ; IA32_EFER
;     rdmsr
;     or eax, 1              ; set LME
;     wrmsr

;     ; --- serial: '4' ---
;     mov dx, 0x3F8
;     mov al, '4'
;     out dx, al

;     ; --- load PML4 into CR3 ---
;     mov eax, pml4_table
;     mov cr3, eax

;     ; --- serial: '5' ---
;     mov dx, 0x3F8
;     mov al, '5'
;     out dx, al

;     ; --- enable paging (PG=1) ---
;     mov eax, cr0
;     or eax, 0x80000000     ; PG
;     mov cr0, eax

;     ; --- serial: '6' ---
;     mov dx, 0x3F8
;     mov al, '6'
;     out dx, al

;     ; NOW load the 64-bit GDT
;     lgdt [gdt64_ptr]

;     ; ; serial: '7' (after lgdt)
;     ; mov dx, 0x3F8
;     ; mov al, '7'
;     ; out dx, al

;     ; --- far jump into 64-bit code segment ---
;     jmp 0x08:kernel_entry

; hang:
;     hlt
;     jmp hang


; ; -----------------------------
; ; 32-bit GDT (normal protected mode)
; ; -----------------------------
; align 8
; gdt32:
;     dq 0                        ; null

;     ; 0x08: 32-bit code segment, base=0, limit=4GB, D=1, L=0
;     dq 0x00CF9A000000FFFF

;     ; 0x10: 32-bit data segment, base=0, limit=4GB, D=1, L=0
;     dq 0x00CF92000000FFFF
; gdt32_end:

; align 8
; gdt32_ptr:
;     dw gdt32_end - gdt32 - 1
;     dd gdt32                   ; 32-bit base is enough for lgdt in 32-bit mode

; ; -----------------------------
; ; 64-bit GDT (for long mode)
; ; -----------------------------
; align 8
; gdt64:
;     dq 0                        ; null
;     dq 0x00209A0000000000       ; 64-bit code (L=1, D=0)
;     dq 0x0000920000000000       ; data
; gdt64_end:

; align 8
; gdt64_ptr:
;     dw gdt64_end - gdt64 - 1
;     dq gdt64                    ; 32-bit lgdt will use low 4 bytes
