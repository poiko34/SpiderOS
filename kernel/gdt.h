#pragma once
#include <stdint.h>

#define GDT_KERNEL_CODE 1
#define GDT_KERNEL_DATA 2
#define GDT_TSS         3

#define GDT_USER_CODE 4
#define GDT_USER_DATA 5

#define KERNEL_CS (GDT_KERNEL_CODE << 3)
#define KERNEL_DS (GDT_KERNEL_DATA << 3)

#define USER_CS ((GDT_USER_CODE << 3) | 3)  // 0x23
#define USER_DS ((GDT_USER_DATA << 3) | 3)  // 0x2B

#define TSS_SEL   (GDT_TSS << 3)

void gdt_init(void);
void gdt_set_gate(int idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);