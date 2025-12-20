#pragma once
#include <stddef.h>
#include <stdint.h>

void vga_clear(void);
void vga_putc(char c);
void vga_print(const char* s);
void vga_println(const char* s);
void vga_setcolor(uint8_t fg, uint8_t bg);
void vga_set_cursor(size_t row, size_t col);
void vga_get_cursor(size_t* r, size_t* c);
void vga_move_cursor(int dr, int dc);
void vga_cursor_blink(void);
void vga_newline_prompt(void);
void vga_put_at(char c, size_t row, size_t col);

void vga_cursor_show(void);
void vga_cursor_hide(void);

void vga_print_hex32(uint32_t v);
void vga_print_dec(uint32_t v);