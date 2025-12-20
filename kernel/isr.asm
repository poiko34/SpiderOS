[bits 32]
global irq0_stub
extern timer_tick

irq0_stub:
    pusha
    call timer_tick
    mov al, 0x20
    out 0x20, al
    popa
    iret
