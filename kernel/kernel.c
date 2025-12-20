#include <stdint.h>
#include "vga.h"
#include "kbd.h"
#include "shell.h"
#include "idt.h"
#include "timer.h"
#include "gdt.h"
#include "paging.h"
#include "memory.h"
#include "pmm.h"
#include "heap.h"
#include "tty.h"

#define MULTIBOOT_MAGIC 0x2BADB002

static tty_t tty0;

static void tty0_write_char(char c) { vga_putc(c); }
static void tty0_put_at(char c, size_t r, size_t col) { vga_put_at(c, r, col); }
static void tty0_clear(void) { vga_clear(); }
static void tty0_set_cursor(size_t r, size_t col) { vga_set_cursor(r, col); }
static void tty0_get_cursor(size_t* r, size_t* col) { vga_get_cursor(r, col); }
static void tty0_cursor_show(void) { vga_cursor_show(); }
static void tty0_cursor_hide(void) { vga_cursor_hide(); }
static void tty0_cursor_blink(void) { vga_cursor_blink(); }

extern uint8_t _kernel_end;

void kmain(uint32_t magic, uint32_t mb_info) {
    (void)mb_info;
    struct tty_ops ops = {
        .write_char   = tty0_write_char,
        .put_at       = tty0_put_at,
        .clear        = tty0_clear,
        .set_cursor   = tty0_set_cursor,
        .get_cursor   = tty0_get_cursor,
        .cursor_show  = tty0_cursor_show,
        .cursor_hide  = tty0_cursor_hide,
        .cursor_blink = tty0_cursor_blink,
    };

    tty_init(&tty0, ops, 80, 25);
    tty_set_active(&tty0);
    vga_clear();

    // 1) Базовая защита CPU
    gdt_init();

    // 2) Paging (identity)
    paging_init();
    vga_println("PAGING: OK");

    // 3) Memory map (multiboot)
    memory_init(mb_info);

    // 4) Проверка multiboot
    if (magic != MULTIBOOT_MAGIC) {
        vga_println("Boot error: not multiboot!");
        for (;;);
    }

    // 5) ИНИЦИАЛИЗАЦИЯ PMM — ТОЛЬКО ЗДЕСЬ
    uint32_t pmm_start = ((uint32_t)&_kernel_end + 0xFFF) & ~0xFFF;
    uint32_t pmm_size  = 0x07EE0000; // временно, из mmap

    pmm_init(pmm_start, pmm_size);

    uint32_t heap_start = pmm_start + 0x20000; // 128KB запас
    heap_init(heap_start, 8 * 1024 * 1024);   // 8MB heap-limit
    
    // 6) ТЕСТ PMM
    uint32_t a = pmm_alloc_frame();
    uint32_t b = pmm_alloc_frame();

    vga_print("alloc A = ");
    vga_print_hex32(a);
    vga_print(" B = ");
    vga_print_hex32(b);
    vga_println("");

    char* x = (char*)kmalloc(64);
    char* y = (char*)kmalloc(64);

    vga_print("x="); vga_print_hex32((uint32_t)(uintptr_t)x);
    vga_print(" y="); vga_print_hex32((uint32_t)(uintptr_t)y); vga_println("");

    kfree(x);
    char* z = (char*)kmalloc(32);

    vga_print("z="); vga_print_hex32((uint32_t)(uintptr_t)z); vga_println("");

    // 7) Дальше — как было
    vga_setcolor(15, 0);

    vga_println("Boot OK. Welcome to SpiderOS!");
    vga_println("Type 'help'.");

    idt_init();
    timer_install();
    pit_init(100);
    kbd_install();
    __asm__ volatile ("sti");

    shell_run();
}
