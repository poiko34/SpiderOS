#include "shell.h"
#include "timer.h"
#include "vga.h"
#include "kbd.h"
#include "io.h"
#include <stddef.h>
#include <stdint.h>

#define PROMPT "> "
#define PROMPT_LEN 2
#define HIST_MAX   16
#define LINE_MAX   128

static char  g_line[128];
static int   g_len = 0;
static int   g_cur = 0;
static int   g_ready = 0;
static size_t g_start = PROMPT_LEN;

static char history[HIST_MAX][LINE_MAX];
static int  hist_count = 0;
static int  hist_pos   = -1;

static void redraw(const char* buf, int len, int cur, size_t start);

static int streq(const char* a, const char* b) {
  while (*a && *b) {
    if (*a != *b) return 0;
    a++; b++;
  }
  return *a == 0 && *b == 0;
}

static void history_add(const char* line)
{
    if (!line[0]) return;

    int idx = hist_count % HIST_MAX;
    for (int i = 0; i < LINE_MAX; i++) {
        history[idx][i] = line[i];
        if (!line[i]) break;
    }
    hist_count++;
}

static void history_load(int index)
{
    if (index < 0 || index >= hist_count) return;

    int idx = index % HIST_MAX;

    g_len = 0;
    g_cur = 0;

    while (history[idx][g_len] && g_len < LINE_MAX - 1) {
        g_line[g_len] = history[idx][g_len];
        g_len++;
    }
    g_line[g_len] = 0;
    g_cur = g_len;

    redraw(g_line, g_len, g_cur, g_start);
}

static void redraw(const char* buf, int len, int cur, size_t start) {
    size_t row;
    vga_get_cursor(&row, NULL);

    vga_cursor_hide();

    for (int i = 0; i < len; i++)
        vga_put_at(buf[i], row, start + (size_t)i);

    for (int i = len; i < (int)(80 - start); i++)
        vga_put_at(' ', row, start + (size_t)i);

    vga_set_cursor(row, start + (size_t)cur);
    vga_cursor_show();
}

/* =========================
   Новая event-driven линия ввода
   ========================= */

static void shell_reset_line(void) {
    g_len = 0;
    g_cur = 0;
    g_ready = 0;
    g_line[0] = 0;
}

/* Вызывается из IRQ-клавиатуры (kbd -> shell_input) */
void shell_input(char c)
{
    if (g_ready) return; // уже есть готовая строка, игнорируем ввод до обработки

    // Enter
    if (c == '\n') {
        g_line[g_len] = 0;
        vga_putc('\n');

        history_add(g_line);
        hist_pos = hist_count;

        g_ready = 1;
        return;
    }

    // Backspace
    if (c == '\b') {
        if (g_cur > 0) {
            for (int i = g_cur - 1; i < g_len - 1; i++)
                g_line[i] = g_line[i + 1];
            g_cur--;
            g_len--;
            redraw(g_line, g_len, g_cur, g_start);
        }
        return;
    }

    // Пока без стрелок: их добавим, когда в kbd будет KEY_LEFT/RIGHT через ext(E0)
    // Можно зарезервировать “спец-коды” позже.

    // Печатаемые
    if ((unsigned char)c >= 32 && (unsigned char)c <= 126) {
        if (g_len < (int)sizeof(g_line) - 1) {
            for (int i = g_len; i > g_cur; i--)
                g_line[i] = g_line[i - 1];
            g_line[g_cur++] = c;
            g_len++;
            redraw(g_line, g_len, g_cur, g_start);
        }
        return;
    }
}

/* Ждём, пока shell_input соберёт строку */
static void shell_wait_line(char* out, int out_max)
{
    // мигание курсора + ожидание IRQ (timer/keyboard)
    uint32_t last_phase = blink_phase;

    while (!g_ready) {
        if (blink_phase != last_phase) {
            last_phase = blink_phase;
            vga_cursor_blink();
        }
        __asm__ volatile("hlt");
    }

    // копируем строку наружу
    int n = g_len;
    if (n > out_max - 1) n = out_max - 1;
    for (int i = 0; i < n; i++) out[i] = g_line[i];
    out[n] = 0;

    shell_reset_line();
}

void shell_run(void) {
    char line[128];

    shell_reset_line();

    while (1) {
        vga_newline_prompt();
        vga_print(PROMPT);
        
        hist_pos = hist_count;
        // ждём строку, собранную через IRQ
        shell_wait_line(line, (int)sizeof(line));

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

void shell_key(int key)
{
    size_t row;
    vga_get_cursor(&row, NULL);

    switch (key) {
        case KEY_LEFT:
            if (g_cur > 0) {
                g_cur--;
                vga_set_cursor(row, g_start + (size_t)g_cur);
            }
            break;

        case KEY_RIGHT:
            if (g_cur < g_len) {
                g_cur++;
                vga_set_cursor(row, g_start + (size_t)g_cur);
            }
            break;

        case KEY_UP:
            if (hist_count == 0) break;
            if (hist_pos > 0) hist_pos--;
            history_load(hist_pos);
            break;

        case KEY_DOWN:
            if (hist_count == 0) break;
            if (hist_pos < hist_count - 1) {
                hist_pos++;
                history_load(hist_pos);
            } else {
                hist_pos = hist_count;
                g_len = g_cur = 0;
                g_line[0] = 0;
                redraw(g_line, 0, 0, g_start);
            }
            break;
    }
}