#include "timer.h"
#include "io.h"

#define PIT_FREQ 1193182

volatile uint32_t timer_ticks = 0;
volatile uint32_t blink_phase = 0;

void pit_init(uint32_t hz) {
    uint32_t div = PIT_FREQ / hz;
    outb(0x43, 0x36);
    outb(0x40, div & 0xFF);
    outb(0x40, (div >> 8) & 0xFF);
}

void timer_tick(void) {
    timer_ticks++;
    if ((timer_ticks % 50) == 0) {   // ~2 раза/сек при 100 Hz
        blink_phase ^= 1;
    }
}