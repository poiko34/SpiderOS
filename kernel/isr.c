#include "isr.h"
#include "vga.h"
#include "io.h"

// Таблица обработчиков
static isr_t interrupt_handlers[256] = {0};

void register_interrupt_handler(uint8_t n, isr_t handler)
{
    interrupt_handlers[n] = handler;
}

// ISR (исключения CPU 0..31)
void isr_handler(regs_t* r)
{
    vga_print("EXC ");
    vga_print_dec(r->int_no);
    vga_print(" err=");
    vga_print_hex32(r->err_code);
    vga_print(" eip=");
    vga_print_hex32(r->eip);
    vga_print(" cr2=");
    uint32_t cr2;
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    vga_print_hex32(cr2);
    vga_println("");

    for(;;) __asm__ volatile("hlt");
}

static inline void pic_eoi(uint8_t irq)
{
    if (irq >= 8) outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

void irq_handler(regs_t* r)
{
    // ack PIC
    uint8_t irq = (uint8_t)(r->int_no - 32);
    pic_eoi(irq);

    // потом уже диспатчишь обработчики
}
