BITS 32
GLOBAL gdt_flush

; void gdt_flush(uint32_t gdt_ptr_addr)
gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]

    ; перезагрузить сегменты данными (селектор 0x10 = индекс 2 * 8)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; far jump для обновления CS (селектор 0x08 = индекс 1 * 8)
    jmp 0x08:.flush_cs

.flush_cs:
    ret
