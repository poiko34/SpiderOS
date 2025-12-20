#include "vga.h"
#include "io.h"

static volatile uint16_t* const VGA = (uint16_t*)0xB8000;
static size_t row = 0, col = 0;
static uint8_t color = 0x0F;
static int cursor_visible = 0;
static int cursor_enabled = 1;

static int cursor_on = 0;
static size_t cursor_r = 0;
static size_t cursor_c = 0;
static uint16_t cursor_saved = 0;

static inline uint16_t vga_entry(char c, uint8_t colr) {
    return (uint16_t)c | ((uint16_t)colr << 8);
}

static void hw_cursor_disable(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

static void hw_cursor_update(void) {
    uint16_t pos = row * 80 + col;
    outb(0x3D4, 0x0F);
    outb(0x3D5, pos & 0xFF);
    outb(0x3D4, 0x0E);
    outb(0x3D5, (pos >> 8) & 0xFF);
}

static void vga_scroll(void) {
    // сдвигаем строки 1..24 → 0..23
    for (size_t y = 1; y < 25; y++) {
        for (size_t x = 0; x < 80; x++) {
            VGA[(y - 1) * 80 + x] = VGA[y * 80 + x];
        }
    }

    // очищаем последнюю строку
    for (size_t x = 0; x < 80; x++) {
        VGA[(25 - 1) * 80 + x] = vga_entry(' ', color);
    }

    row = 24;
}

void vga_cursor_show(void) {
    if (cursor_on) return;

    vga_get_cursor(&cursor_r, &cursor_c);
    size_t idx = cursor_r * 80 + cursor_c;

    cursor_saved = VGA[idx];
    uint8_t attr = cursor_saved >> 8;
    uint8_t fg = attr & 0x0F;
    uint8_t bg = (attr >> 4) & 0x0F;

    uint8_t inv = (fg << 4) | bg;
    VGA[idx] = (cursor_saved & 0x00FF) | (inv << 8);

    cursor_on = 1;
    cursor_visible = 1;
}

void vga_cursor_hide(void) {
    cursor_visible = 0;
    if (!cursor_on) return;

    size_t idx = cursor_r * 80 + cursor_c;
    VGA[idx] = cursor_saved;
    cursor_on = 0;
}

void vga_cursor_blink(void) {
    if (!cursor_enabled) return;

    if (cursor_visible)
        vga_cursor_hide();
    else
        vga_cursor_show();
}

void vga_set_cursor(size_t r, size_t c) {
    vga_cursor_hide();
    row = r;
    col = c;
    vga_cursor_show();
}

void vga_get_cursor(size_t* r, size_t* c) {
    if (r) *r = row;
    if (c) *c = col;
}

void vga_move_cursor(int dr, int dc) {
    int nr = (int)row + dr;
    int nc = (int)col + dc;

    if (nc < 0) { nc = 0; }
    if (nc >= 80) { nc = 79; }
    if (nr < 0) { nr = 0; }
    if (nr >= 25) { nr = 24; }

    row = (size_t)nr;
    col = (size_t)nc;
    hw_cursor_update();
}

void vga_clear(void) {
    hw_cursor_disable();

    for (size_t y = 0; y < 25; y++)
        for (size_t x = 0; x < 80; x++)
            VGA[y * 80 + x] = vga_entry(' ', color);

    vga_set_cursor(0, 0);
}

void vga_putc(char c) {
    vga_cursor_hide();

    if (c == '\n') {
        col = 0;
        row++;
        if (row >= 25)
            vga_scroll();
        vga_cursor_show();
        return;
    }

    VGA[row * 80 + col] = vga_entry(c, color);
    col++;

    if (col >= 80) {
        col = 0;
        row++;
        if (row >= 25)
            vga_scroll();
    }

    vga_cursor_show();
}


void vga_setcolor(uint8_t fg, uint8_t bg) {
    color = (bg << 4) | (fg & 0x0F);
}

void vga_print(const char* s) {
    while (*s)
        vga_putc(*s++);
}

void vga_println(const char* s) {
    vga_print(s);
    vga_putc('\n');
}

void vga_newline_prompt(void) {
    col = 0;
    if (row >= 25)
        vga_scroll();
    vga_set_cursor(row, 0);
}

void vga_put_at(char c, size_t r, size_t c2) {
    VGA[r * 80 + c2] = vga_entry(c, color);
}

void vga_print_hex32(uint32_t v) {
    const char* hex = "0123456789ABCDEF";
    vga_print("0x");
    for (int i = 7; i >= 0; --i) {
        uint8_t nib = (v >> (i * 4)) & 0xF;
        vga_putc(hex[nib]);
    }
}

void vga_print_dec(uint32_t v) {
    char buf[11];
    int i = 0;
    if (v == 0) { vga_putc('0'); return; }
    while (v > 0 && i < 10) {
        buf[i++] = (char)('0' + (v % 10));
        v /= 10;
    }
    while (i--) vga_putc(buf[i]);
}