#include "shell.h"
#include "timer.h"
#include "vga.h"
#include "kbd.h"
#include "io.h"
#include <stddef.h>

#define PROMPT "> "
#define PROMPT_LEN 2

static int streq(const char* a, const char* b) {
  while (*a && *b) {
    if (*a != *b) return 0;
    a++; b++;
  }
  return *a == 0 && *b == 0;
}

static void redraw(const char* buf, int len, int cur, size_t start) {
    size_t row;
    vga_get_cursor(&row, NULL);

    vga_cursor_hide();

    for (int i = 0; i < len; i++)
        vga_put_at(buf[i], row, start + i);

    for (int i = len; i < 80 - start; i++)
        vga_put_at(' ', row, start + i);

    vga_set_cursor(row, start + cur);
    vga_cursor_show();
}


static void read_line(char* buf, int max) {
    int len = 0;
    int cur = 0;

    size_t start = PROMPT_LEN;
    uint32_t last_phase = blink_phase;

    while (1) {

        if (blink_phase != last_phase) {
            last_phase = blink_phase;
            vga_cursor_blink();
        }

        int k = kbd_getkey_nb();
        if (!k)
            continue;

        if (k == '\n') {
            buf[len] = 0;
            vga_putc('\n');
            return;
        }

        if (k == '\b' && cur > 0) {
            for (int i = cur - 1; i < len - 1; i++)
                buf[i] = buf[i + 1];
            cur--;
            len--;
            redraw(buf, len, cur, start);
            continue;
        }

        if (k == KEY_LEFT && cur > 0) {
            cur--;
            size_t row;
            vga_get_cursor(&row, NULL);
            vga_set_cursor(row, start + cur);
            continue;
        }

        if (k == KEY_RIGHT && cur < len) {
            cur++;
            size_t row;
            vga_get_cursor(&row, NULL);
            vga_set_cursor(row, start + cur);
            continue;
        }

        if (k >= 32 && k <= 126 && len < max - 1) {
            for (int i = len; i > cur; i--)
                buf[i] = buf[i - 1];
            buf[cur++] = (char)k;
            len++;
            redraw(buf, len, cur, start);
        }
    }
}

void shell_run(void) {
    char line[128];
    uint32_t last_phase = blink_phase;
    
    while (1) {

        // idle: мигание курсора
        if (blink_phase != last_phase) {
            last_phase = blink_phase;
            vga_cursor_blink();
        }

        vga_newline_prompt();
        vga_print(PROMPT);
        read_line(line, sizeof(line));

        if (streq(line, "help")) {
            vga_println("Commands: help, clear, about");
        } else if (streq(line, "clear")) {
            vga_clear();
        } else if (streq(line, "about")) {
            vga_println("spiderOS kernel");
        } else if (line[0]) {
            vga_print("Unknown: ");
            vga_println(line);
        }
    }
}

void shell_input(char c)
{
    // ВРЕМЕННО: просто печать, чтобы проверить IRQ-клавиатуру
    if (c) vga_putc(c);
}