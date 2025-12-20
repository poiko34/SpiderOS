#include "gdt.h"

extern void gdt_flush(uint32_t);

struct __attribute__((packed)) gdt_entry {
    uint16_t limit_low;     // bits 0..15 of limit
    uint16_t base_low;      // bits 0..15 of base
    uint8_t  base_mid;      // bits 16..23 of base
    uint8_t  access;        // access flags
    uint8_t  gran;          // granularity + high limit
    uint8_t  base_high;     // bits 24..31 of base
};

struct __attribute__((packed)) gdt_ptr {
    uint16_t limit;
    uint32_t base;
};

static struct gdt_entry gdt[3];
static struct gdt_ptr gp;

static void gdt_set(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[i].base_low  = (uint16_t)(base & 0xFFFF);
    gdt[i].base_mid  = (uint8_t)((base >> 16) & 0xFF);
    gdt[i].base_high = (uint8_t)((base >> 24) & 0xFF);

    gdt[i].limit_low = (uint16_t)(limit & 0xFFFF);
    gdt[i].gran      = (uint8_t)((limit >> 16) & 0x0F);

    gdt[i].gran |= (gran & 0xF0);
    gdt[i].access = access;
}

// объявим ASM функцию, которая сделает lgdt + перезагрузит сегменты
extern void gdt_flush(uint32_t gdt_ptr_addr);

void gdt_init(void) {
    gp.limit = (uint16_t)(sizeof(gdt) - 1);
    gp.base  = (uint32_t)&gdt;

    // 0: null descriptor
    gdt_set(0, 0, 0, 0, 0);

    // 1: kernel code: base=0, limit=4GB, access=0x9A, gran=0xCF
    // access: 1 00 1 1010 = present, ring0, code, readable
    // gran:   1100 1111 = 4K gran + 32-bit + high limit bits
    gdt_set(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // 2: kernel data: access=0x92
    gdt_set(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    gdt_flush((uint32_t)&gp);
}
