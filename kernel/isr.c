#include "isr.h"
#include "vga.h"
#include "io.h"

static isr_t handlers[256] = {0};

void register_interrupt_handler(uint8_t n, isr_t handler) {
    handlers[n] = handler;
}

static inline void pic_eoi(uint8_t irq) {
    if (irq >= 8) outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

static const char* exc_name(uint32_t n) {
    static const char* names[32] = {
        "Divide Error", "Debug", "NMI", "Breakpoint",
        "Overflow", "BOUND Range", "Invalid Opcode", "Device Not Avail",
        "Double Fault", "Coprocessor Seg Overrun", "Invalid TSS", "Segment Not Present",
        "Stack-Segment Fault", "General Protection", "Page Fault", "Reserved",
        "x87 FP Exception", "Alignment Check", "Machine Check", "SIMD FP Exception",
        "Virtualization", "Control Protection", "Reserved", "Reserved",
        "Reserved", "Reserved", "Reserved", "Reserved",
        "Reserved", "Reserved", "Reserved", "Reserved"
    };
    return (n < 32) ? names[n] : "Unknown";
}

static void dump_regs(regs_t* r) {
    vga_print("EAX="); vga_print_hex32(r->eax);
    vga_print(" EBX="); vga_print_hex32(r->ebx);
    vga_print(" ECX="); vga_print_hex32(r->ecx);
    vga_print(" EDX="); vga_print_hex32(r->edx);
    vga_println("");

    vga_print("ESI="); vga_print_hex32(r->esi);
    vga_print(" EDI="); vga_print_hex32(r->edi);
    vga_print(" EBP="); vga_print_hex32(r->ebp);
    vga_print(" ESP="); vga_print_hex32(r->esp);
    vga_println("");

    vga_print("EIP="); vga_print_hex32(r->eip);
    vga_print(" CS="); vga_print_hex32(r->cs);
    vga_print(" EFL="); vga_print_hex32(r->eflags);
    vga_println("");
}

static void pf_print_flags(uint32_t err) {
    // err bits:
    // 0 P   0=not-present, 1=protection violation
    // 1 W/R 0=read, 1=write
    // 2 U/S 0=kernel, 1=user
    // 3 RSVD 1=reserved bit violation
    // 4 I/D 1=instruction fetch
    vga_print(" [");
    vga_print("P="); vga_print_dec(err & 1);
    vga_print(" W="); vga_print_dec((err >> 1) & 1);
    vga_print(" U="); vga_print_dec((err >> 2) & 1);
    vga_print(" R="); vga_print_dec((err >> 3) & 1);
    vga_print(" I="); vga_print_dec((err >> 4) & 1);
    vga_print("] ");
}

static void pf_print_human(uint32_t err) {
    const int present = (err & 1);
    const int write   = (err >> 1) & 1;
    const int user    = (err >> 2) & 1;
    const int rsvd    = (err >> 3) & 1;
    const int instr   = (err >> 4) & 1;

    // кратко, но очень информативно
    vga_print(present ? "PROT " : "NP ");
    vga_print(write ? "WRITE " : "READ ");
    vga_print(user ? "USER " : "KERN ");
    if (rsvd)  vga_print("RSVD ");
    if (instr) vga_print("IFETCH ");
}

void isr_handler(regs_t* r) {
    if (handlers[r->int_no]) {
        handlers[r->int_no](r);
        return;
    }

    vga_setcolor(15, 4); // белый на красном (если есть)
    vga_print("EXCEPTION ");
    vga_print_dec(r->int_no);
    vga_print(": ");
    vga_println(exc_name(r->int_no));
    vga_print("err="); vga_print_hex32(r->err_code);

    if (r->int_no == 14) {
        uint32_t cr2;
        __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));

        vga_print(" cr2=");
        vga_print_hex32(cr2);
        vga_print(" err=");
        vga_print_hex32(r->err_code);

        pf_print_flags(r->err_code);
        pf_print_human(r->err_code);

        vga_println("");
    }
    vga_println("");

    dump_regs(r);

    for (;;) __asm__ volatile("hlt");
}

void irq_handler(regs_t* r) {
    uint8_t irq = (uint8_t)(r->int_no - 32);

    // Сначала диспатч, потом EOI — можно и наоборот,
    // но так удобнее ловить "залипание" если хендлер завис.
    if (handlers[r->int_no]) {
        handlers[r->int_no](r);
    }

    pic_eoi(irq);
}
