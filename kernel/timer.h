#pragma once
#include <stdint.h>

void pit_init(uint32_t hz);
void timer_tick(void);
void vga_cursor_blink(void);
void timer_install(void);

uint32_t timer_get_ticks(void);

extern volatile uint32_t blink_phase;
