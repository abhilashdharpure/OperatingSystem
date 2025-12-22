[BITS 32]
section .boot32

global multiboot_entry
extern pml4_table
extern kernel_entry

multiboot_entry:
    cli

    mov dx, 0x3F8
    mov al, '1'
    out dx, al

    ; load unified GDT
    lgdt [gdt_ptr]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov dx, 0x3F8
    mov al, '2'
    out dx, al

    ; --- enable PAE ---
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    mov dx, 0x3F8
    mov al, '3'
    out dx, al

    ; --- enable LME ---
    mov ecx, 0xC0000080        ; IA32_EFER
    rdmsr
    or eax, 1                  ; set LME
    wrmsr

    mov dx, 0x3F8
    mov al, '4'
    out dx, al

    ; --- load PML4 (identity-mapped) ---
    mov eax, pml4_table        ; physical address of PML4
    mov cr3, eax

    mov dx, 0x3F8
    mov al, '5'
    out dx, al

    ; --- enable paging (PG=1) ---
    mov eax, cr0
    or eax, 0x80000000         ; PG
    mov cr0, eax

    mov dx, 0x3F8
    mov al, '6'
    out dx, al

    ; now we are in IA-32e, compatibility mode (still 32-bit CS)
    mov dx, 0x3F8
    mov al, '7'
    out dx, al

    ; far jump into 64-bit code segment (0x08)
    ; jmp 0x08:long_mode_entry
    jmp long_mode_entry


; unified GDT for long mode
align 8
gdt:
    dq 0                        ; null

    ; 0x08: 64-bit kernel code segment (L=1, D=0)
    dq 0x00209A0000000000

    ; 0x10: 64-bit kernel data segment (L=0, D=0)
    dq 0x0000920000000000

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
; section .boot32

; global multiboot_entry
; extern pml4_table
; extern kernel_entry

; multiboot_entry:
;     cli

;     mov dx, 0x3F8
;     mov al, '1'
;     out dx, al

;     lgdt [gdt_ptr]

;     mov ax, 0x10
;     mov ds, ax
;     mov es, ax
;     mov ss, ax

;     mov dx, 0x3F8
;     mov al, '2'
;     out dx, al

;     ; enable PAE
;     mov eax, cr4
;     or eax, 1 << 5
;     mov cr4, eax

;     mov dx, 0x3F8
;     mov al, '3'
;     out dx, al

;     ; enable LME
;     mov ecx, 0xC0000080
;     rdmsr
;     or eax, 1
;     wrmsr

;     mov dx, 0x3F8
;     mov al, '4'
;     out dx, al

;     ; load PML4 (identity-mapped)
;     mov eax, pml4_table        ; MUST be physical address
;     mov cr3, eax

;     mov dx, 0x3F8
;     mov al, '5'
;     out dx, al

;     ; enable paging
;     mov eax, cr0
;     or eax, 0x80000000
;     mov cr0, eax

;     mov dx, 0x3F8
;     mov al, '6'
;     out dx, al

;     mov dx, 0x3F8
;     mov al, '7'
;     out dx, al

;     ; ; far jump into 64-bit code segment (0x08)
;     ; jmp 0x08:long_mode_entry

;     mov eax, long_mode_entry    ; linear address
;     ; print a marker so we know this ran
;     mov dx, 0x3F8
;     mov al, 'A'
;     out dx, al
;     ; optionally dump eax as hex via serial_putc_asm, or just trust it's low




; ; unified GDT (correct long-mode descriptors)
; align 8
; gdt:
;     dq 0                        ; null

;     ; 0x08: 64-bit kernel code segment (L=1, D=0)
;     dq 0x00209A0000000000

;     ; 0x10: 64-bit kernel data segment (L=0, D=0)
;     dq 0x0000920000000000

; gdt_end:

; align 8
; gdt_ptr:
;     dw gdt_end - gdt - 1
;     dd gdt


; [BITS 64]
; long_mode_entry:
;     mov dx, 0x3F8
;     mov al, 'L'
;     out dx, al

;     jmp kernel_entry
