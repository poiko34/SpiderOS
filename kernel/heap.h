#pragma once
#include <stdint.h>
#include <stddef.h>

void heap_init(uint32_t heap_start_phys, uint32_t heap_size_bytes);

void* kmalloc(size_t size);
void* kmalloc_aligned(size_t size, size_t align);
void* kmalloc_page_aligned(size_t size);

void kfree(void* ptr);
