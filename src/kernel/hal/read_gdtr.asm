; ; read_gdtr.asm
; [BITS 64]
; global read_gdtr

; ; void read_gdtr(uint64_t *low, uint64_t *high);
; ;   low  in RDI
; ;   high in RSI

; read_gdtr:
;     sub     rsp, 16          ; make space (we only need 10 bytes, 16 is aligned)
;     sgdt    [rsp]            ; store 10-byte GDTR: limit(2) + base(8)

;     ; Read the 10 bytes as two 64-bit chunks for debugging.
;     ; low  = first 8 bytes: limit (16 bits) + first 48 bits of base
;     ; high = last 2 bytes:  remaining top 16 bits of base (we'll pack them)
;     mov     rax, [rsp]       ; bytes 0..7
;     mov     rdx, [rsp+8]     ; bytes 8..15 (top 2 bytes meaningful)

;     mov     [rdi], rax       ; *low  = rax
;     mov     [rsi], rdx       ; *high = rdx

;     add     rsp, 16
;     ret
