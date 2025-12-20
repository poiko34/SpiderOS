#include "isr.h"
#include "log.h"
#include "io.h"
#include "panic.h"

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
    klog("EAX="); klog_hex32(r->eax);
    klog(" EBX="); klog_hex32(r->ebx);
    klog(" ECX="); klog_hex32(r->ecx);
    klog(" EDX="); klog_hex32(r->edx);
    klogln("");

    klog("ESI="); klog_hex32(r->esi);
    klog(" EDI="); klog_hex32(r->edi);
    klog(" EBP="); klog_hex32(r->ebp);
    klog(" ESP="); klog_hex32(r->esp);
    klogln("");

    klog("EIP="); klog_hex32(r->eip);
    klog(" CS="); klog_hex32(r->cs);
    klog(" EFL="); klog_hex32(r->eflags);
    klogln("");
}

static void pf_print_flags(uint32_t err) {
    // err bits:
    // 0 P   0=not-present, 1=protection violation
    // 1 W/R 0=read, 1=write
    // 2 U/S 0=kernel, 1=user
    // 3 RSVD 1=reserved bit violation
    // 4 I/D 1=instruction fetch
    klog(" [");
    klog("P="); klog_dec(err & 1);
    klog(" W="); klog_dec((err >> 1) & 1);
    klog(" U="); klog_dec((err >> 2) & 1);
    klog(" R="); klog_dec((err >> 3) & 1);
    klog(" I="); klog_dec((err >> 4) & 1);
    klog("] ");
}

static void pf_print_human(uint32_t err) {
    const int present = (err & 1);
    const int write   = (err >> 1) & 1;
    const int user    = (err >> 2) & 1;
    const int rsvd    = (err >> 3) & 1;
    const int instr   = (err >> 4) & 1;

    // кратко, но очень информативно
    klog(present ? "PROT " : "NP ");
    klog(write ? "WRITE " : "READ ");
    klog(user ? "USER " : "KERN ");
    if (rsvd)  klog("RSVD ");
    if (instr) klog("IFETCH ");
}

void isr_handler(regs_t* r) {
    if (handlers[r->int_no]) {
        handlers[r->int_no](r);
        return;
    }

    // vga_setcolor(15, 4);
    klog("EXCEPTION ");
    klog_dec(r->int_no);
    klog(": ");
    klogln(exc_name(r->int_no));
    klog("err="); klog_hex32(r->err_code);

    if (r->int_no == 14) {
        uint32_t cr2;
        __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));

        klog("PF cr2=");
        klog_hex32(cr2);
        klog(" err=");
        klog_hex32(r->err_code);
        klogln("");
    }

    PANIC_REGS("Page Fault", r);
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
