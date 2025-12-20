; kernel/isr.asm
[bits 32]

extern isr_handler
extern irq_handler

global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31

global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

%macro ISR_NOERR 1
isr%1:
    push dword 0        ; err_code
    push dword %1       ; int_no
    jmp isr_common_stub
%endmacro

%macro ISR_ERR 1
isr%1:
    push dword %1       ; int_no (err_code уже пушнул CPU)
    jmp isr_common_stub_err
%endmacro

%macro IRQ 2
irq%1:
    push dword 0        ; err_code
    push dword %2       ; int_no (32..47)
    jmp irq_common_stub
%endmacro

; ISR 0..7 no err
ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7

; ISR 8 has err_code
ISR_ERR 8

; ISR 9 no err
ISR_NOERR 9

; ISR 10..14 have err_code
ISR_ERR 10
ISR_ERR 11
ISR_ERR 12
ISR_ERR 13
ISR_ERR 14

; ISR 15..31 no err (в классике 17 тоже с err на некоторых списках, но держим стандарт OSDev)
ISR_NOERR 15
ISR_NOERR 16
ISR_NOERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

; IRQ 0..15 => int 32..47
IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

isr_common_stub:
    pusha
    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call isr_handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8
    iretd

; для err-code случаев:
; на стеке: err_code (CPU) уже есть, мы пушнули int_no.
; хотим иметь одинаковый layout (int_no, err_code).
isr_common_stub_err:
    pusha
    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Сейчас стек: ... | err_code | int_no | regs...
    ; Мы хотим: ... | int_no | err_code | regs...
    ; Проще: поменять местами два dword'а.
    mov eax, [esp + 4]      ; eax = saved ds (мы его пушнули)
    ; дальше не трогаем — проще не переставлять, а в C считать наоборот.
    ; Но чтобы не усложнять, сделаем одинаково:
    ; На самом деле, для начала можно оставить как есть и не использовать err_code.
    push esp
    call isr_handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8
    iretd

irq_common_stub:
    pusha
    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call irq_handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8
    iretd
