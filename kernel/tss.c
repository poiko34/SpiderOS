#include "tss.h"
#include "gdt.h"
#include "string.h"

static tss_entry_t tss;

extern void tss_flush(uint16_t selector);

/*
  Дескриптор TSS в GDT:
  base = &tss
  limit = sizeof(tss)-1
  access = 0x89 (present, ring0, available 32-bit TSS)
  gran = 0x00 (byte granularity)
*/
static void gdt_set_tss(int idx, uint32_t base, uint32_t limit) {
    // IMPORTANT:
    // В твоём gdt.c уже есть функция gdt_set_gate(...)
    // Ниже — вызов в типичном стиле:
    // gdt_set_gate(idx, base, limit, access, gran);

    gdt_set_gate(idx, base, limit, 0x89, 0x00);
}

void tss_init(uint16_t kernel_ss, uint32_t kernel_esp) {
    memset(&tss, 0, sizeof(tss));

    tss.ss0  = kernel_ss;
    tss.esp0 = kernel_esp;

    tss.cs = 0x1B;
    tss.ss = 0x23;
    tss.ds = 0x23;
    tss.es = 0x23;
    tss.fs = 0x23;
    tss.gs = 0x23;

    tss.iomap_base = sizeof(tss_entry_t);

    uint32_t base  = (uint32_t)&tss;
    uint32_t limit = sizeof(tss_entry_t) - 1;

    gdt_set_tss(GDT_TSS, base, limit);

    // загружаем TR
    tss_flush(TSS_SEL);
}

void tss_set_kernel_stack(uint32_t esp0) {
    tss.esp0 = esp0;
}