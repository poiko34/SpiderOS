#pragma once
#include <stdint.h>

void pmm_init(uint32_t mem_start, uint32_t mem_size);
uint32_t pmm_alloc_frame(void);
void pmm_free_frame(uint32_t phys_addr);
