#include "pmm.h"
#include "vga.h"

#define PAGE_SIZE 4096

static uint32_t* bitmap;
static uint32_t total_pages;
static uint32_t used_pages;

static inline void bitmap_set(uint32_t i) {
    bitmap[i / 32] |= (1u << (i % 32));
}

static inline void bitmap_clear(uint32_t i) {
    bitmap[i / 32] &= ~(1u << (i % 32));
}

static inline int bitmap_test(uint32_t i) {
    return bitmap[i / 32] & (1u << (i % 32));
}

void pmm_init(uint32_t mem_start, uint32_t mem_size)
{
    total_pages = mem_size / PAGE_SIZE;
    used_pages = 0;

    bitmap = (uint32_t*)mem_start;

    uint32_t bitmap_size = (total_pages + 31) / 32;

    // очистим bitmap
    for (uint32_t i = 0; i < bitmap_size; i++)
        bitmap[i] = 0;

    // пометим страницы bitmap как занятые
    uint32_t bitmap_pages = (bitmap_size * 4 + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint32_t i = 0; i < bitmap_pages; i++) {
        bitmap_set(i);
        used_pages++;
    }

    vga_print("PMM pages total: ");
    vga_print_dec(total_pages);
    vga_println("");
}

uint32_t pmm_alloc_frame(void)
{
    for (uint32_t i = 0; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            used_pages++;
            return i * PAGE_SIZE;
        }
    }
    return 0; // out of memory
}

void pmm_free_frame(uint32_t phys_addr)
{
    uint32_t i = phys_addr / PAGE_SIZE;
    bitmap_clear(i);
    used_pages--;
}
