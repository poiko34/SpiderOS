#include "idt.h"
#include "io.h"

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr idtp;

static uint16_t get_cs(void) {
    uint16_t cs;
    __asm__ volatile ("mov %%cs, %0" : "=r"(cs));
    return cs;
}

static void idt_set_gate_flags(int n, uint32_t handler, uint8_t flags) {
    idt[n].offset_low  = handler & 0xFFFF;
    idt[n].selector    = get_cs();
    idt[n].zero        = 0;
    idt[n].type_attr   = flags;
    idt[n].offset_high = (handler >> 16) & 0xFFFF;
}

static void idt_set_gate(int n, uint32_t handler) {
    idt[n].offset_low  = handler & 0xFFFF;
    idt[n].selector    = get_cs();   // можно оставить так; после gdt_init это обычно 0x08
    idt[n].zero        = 0;
    idt[n].type_attr   = 0x8E;       // present=1, DPL=0, 32-bit interrupt gate
    idt[n].offset_high = (handler >> 16) & 0xFFFF;
    idt_set_gate_flags(n, handler, 0x8E);
}

static inline void lidt(void* ptr) {
    __asm__ volatile ("lidt (%0)" :: "r"(ptr));
}

static void pic_remap(void) {
    // init
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // offsets
    outb(0x21, 0x20); // master PIC -> int 32
    outb(0xA1, 0x28); // slave  PIC -> int 40

    // wiring
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // 8086 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // MASK:
    // было: enable IRQ0 only (0xFE)
    // но для dispatcher нам нужны хотя бы IRQ0(timer) и IRQ1(kbd)
    outb(0x21, 0xFC); // 11111100 -> IRQ0+IRQ1 enabled
    outb(0xA1, 0xFF); // все IRQ на slave отключены
}

// ====== ISR/IRQ stubs из твоего нового isr.asm ======
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr128();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

#define SET_ISR(n) idt_set_gate((n), (uint32_t)isr##n)
#define SET_IRQ(i) idt_set_gate(32 + (i), (uint32_t)irq##i)

void idt_init(void) {
    // очистка
    for (int i = 0; i < 256; i++) {
        idt[i].offset_low  = 0;
        idt[i].selector    = 0;
        idt[i].zero        = 0;
        idt[i].type_attr   = 0;
        idt[i].offset_high = 0;
    }

    // ===== Exceptions: 0..31 =====
    SET_ISR(0);  SET_ISR(1);  SET_ISR(2);  SET_ISR(3);
    SET_ISR(4);  SET_ISR(5);  SET_ISR(6);  SET_ISR(7);
    SET_ISR(8);  SET_ISR(9);  SET_ISR(10); SET_ISR(11);
    SET_ISR(12); SET_ISR(13); SET_ISR(14); SET_ISR(15);
    SET_ISR(16); SET_ISR(17); SET_ISR(18); SET_ISR(19);
    SET_ISR(20); SET_ISR(21); SET_ISR(22); SET_ISR(23);
    SET_ISR(24); SET_ISR(25); SET_ISR(26); SET_ISR(27);
    SET_ISR(28); SET_ISR(29); SET_ISR(30); SET_ISR(31);

    idt_set_gate_flags(0x80, (uint32_t)isr128, 0xEE);

    // ===== Hardware IRQ: 32..47 =====
    SET_IRQ(0);  SET_IRQ(1);  SET_IRQ(2);  SET_IRQ(3);
    SET_IRQ(4);  SET_IRQ(5);  SET_IRQ(6);  SET_IRQ(7);
    SET_IRQ(8);  SET_IRQ(9);  SET_IRQ(10); SET_IRQ(11);
    SET_IRQ(12); SET_IRQ(13); SET_IRQ(14); SET_IRQ(15);

    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uint32_t)&idt;

    pic_remap();
    lidt(&idtp);
}
