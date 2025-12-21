#include "paging.h"
#include <stdint.h>

// Флаги страниц
#define PAGE_PRESENT 0x001
#define PAGE_RW      0x002
#define PAGE_USER    0x004

// Замапим 16MB = 4 таблицы по 4MB
#define NUM_TABLES 4

static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t page_tables[NUM_TABLES][1024] __attribute__((aligned(4096)));

static inline void load_cr3(uint32_t phys_pd_addr) {
    __asm__ volatile ("mov %0, %%cr3" :: "r"(phys_pd_addr) : "memory");
}

static inline uint32_t read_cr0(void) {
    uint32_t v;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(v));
    return v;
}

static inline void write_cr0(uint32_t v) {
    __asm__ volatile ("mov %0, %%cr0" :: "r"(v) : "memory");
}

void paging_init(void)
{
    // 1) Очистим directory
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0;
    }

    // 2) Заполним таблицы: identity map 0..16MB
    uint32_t phys = 0;
    for (int t = 0; t < NUM_TABLES; t++) {
        for (int i = 0; i < 1024; i++) {
            page_tables[t][i] =
                (phys & 0xFFFFF000)
                | PAGE_PRESENT
                | PAGE_RW
                | PAGE_USER;     // <-- важно для ring3 тестов
            phys += 0x1000; // +4KB
        }
    }

    // 3) Подключим таблицы к directory
    for (int t = 0; t < NUM_TABLES; t++) {
        page_directory[t] =
            ((uint32_t)page_tables[t] & 0xFFFFF000)
            | PAGE_PRESENT
            | PAGE_RW
            | PAGE_USER;         // <-- тоже важно
    }

    // 4) Загрузим CR3 (пока identity map ещё действует)
    load_cr3((uint32_t)page_directory);

    // 5) Включим paging (бит PG)
    uint32_t cr0 = read_cr0();
    cr0 |= 0x80000000u;
    write_cr0(cr0);

    // flush pipeline
    __asm__ volatile ("jmp 1f\n1:");
}
