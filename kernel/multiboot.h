// kernel/multiboot.h
#pragma once
#include <stdint.h>

// Multiboot info flags bits:
#define MB_FLAG_MEM      (1u << 0)  // mem_lower, mem_upper valid
#define MB_FLAG_MMAP     (1u << 6)  // mmap_* valid

typedef struct __attribute__((packed)) multiboot_info {
    uint32_t flags;

    uint32_t mem_lower; // KB
    uint32_t mem_upper; // KB

    uint32_t boot_device;

    uint32_t cmdline;

    uint32_t mods_count;
    uint32_t mods_addr;

    // a.out + elf sections (мы их пропустим по layout)
    uint32_t syms[4];

    uint32_t mmap_length;
    uint32_t mmap_addr;

    // дальше много полей, но нам пока не нужно
} multiboot_info_t;

typedef struct __attribute__((packed)) multiboot_mmap_entry {
    uint32_t size;      // размер структуры без этого поля
    uint64_t addr;      // физический адрес
    uint64_t len;       // длина
    uint32_t type;      // 1=RAM, иначе reserved/ACPI/...
} multiboot_mmap_entry_t;
