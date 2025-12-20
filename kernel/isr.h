#pragma once
#include <stdint.h>

typedef struct regs {
    uint32_t ds;

    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;

    uint32_t int_no, err_code;

    uint32_t eip, cs, eflags, useresp, ss;
} regs_t;

typedef void (*isr_t)(regs_t*);

void register_interrupt_handler(uint8_t n, isr_t handler);
void isr_handler(regs_t* r);
void irq_handler(regs_t* r);
