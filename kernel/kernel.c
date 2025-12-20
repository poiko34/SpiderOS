#include <stdint.h>
#include "vga.h"
#include "kbd.h"
#include "shell.h"
#include "idt.h"
#include "timer.h"

#define MULTIBOOT_MAGIC 0x2BADB002

void kmain(uint32_t magic, uint32_t mb_info) {
    (void)mb_info;

    vga_setcolor(15, 0);
    vga_clear();

    if (magic != MULTIBOOT_MAGIC) {
        vga_println("Boot error: not multiboot!");
        for (;;);
    }

    vga_println("Boot OK. Welcome to SpiderOS!");
    vga_println("Type 'help'.");

    idt_init();
    pit_init(100);

    __asm__ volatile ("sti");

    shell_run();
}
