#include "timer.h"
#include "isr.h"
#include "io.h"

static volatile uint32_t ticks = 0;
volatile uint32_t blink_phase = 0;

static void timer_cb(regs_t* r)
{
    (void)r;
    ticks++;

    // мигание ~2 раза в секунду (если PIT = 100 Hz)
    if ((ticks % 50) == 0) {
        blink_phase++;
    }
}

void pit_init(uint32_t freq)
{
    uint32_t divisor = 1193180 / freq;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}

void timer_install(void)
{
    register_interrupt_handler(32, timer_cb);
}

uint32_t timer_get_ticks(void)
{
    return ticks;
}
