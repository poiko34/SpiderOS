#include "shell.h"
#include "timer.h"
#include "kbd.h"
#include "io.h"
#include "tty.h"
#include <stddef.h>
#include <stdint.h>

#define PROMPT "> "
#define PROMPT_LEN 2
#define HIST_MAX   16
#define LINE_MAX   128

static char   g_line[LINE_MAX];
static int    g_len = 0;
static int    g_cur = 0;
static int    g_ready = 0;
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

/* ---------- tiny tty print helpers ---------- */
static void tty_prints(const char* s) {
    tty_t* t = tty_active();
    while (*s) tty_putc(t, *s++);
}
static void tty_println(const char* s) {
    tty_prints(s);
    tty_putc(tty_active(), '\n');
}

/* ---------- history ---------- */
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

/* ---------- redraw line editor ---------- */
static void redraw(const char* buf, int len, int cur, size_t start)
{
    tty_t* t = tty_active();
    size_t row;
    tty_get_cursor(t, &row, NULL);

    tty_cursor_hide(t);

    for (int i = 0; i < len; i++)
        tty_put_at(t, buf[i], row, start + (size_t)i);

    for (int i = len; i < (int)(t->cols - start); i++)
        tty_put_at(t, ' ', row, start + (size_t)i);

    tty_set_cursor(t, row, start + (size_t)cur);
    tty_cursor_show(t);
}

static void shell_reset_line(void)
{
    g_len = 0;
    g_cur = 0;
    g_ready = 0;
    g_line[0] = 0;
}

/* Вызывается из IRQ-клавиатуры (через tty_input_char -> shell_input) */
void shell_input(char c)
{
    if (g_ready) return;

    // Enter
    if (c == '\n') {
        g_line[g_len] = 0;
        tty_putc(tty_active(), '\n');

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

    // Printable
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

static void shell_wait_line(char* out, int out_max)
{
    uint32_t last_phase = blink_phase;

    while (!g_ready) {
        if (blink_phase != last_phase) {
            last_phase = blink_phase;
            tty_cursor_blink(tty_active());
        }
        __asm__ volatile("hlt");
    }

    int n = g_len;
    if (n > out_max - 1) n = out_max - 1;
    for (int i = 0; i < n; i++) out[i] = g_line[i];
    out[n] = 0;

    shell_reset_line();
}

void shell_run(void)
{
    char line[LINE_MAX];

    shell_reset_line();

    while (1) {
        // новый prompt
        tty_putc(tty_active(), '\n');
        tty_prints(PROMPT);

        hist_pos = hist_count;

        shell_wait_line(line, (int)sizeof(line));

        if (streq(line, "help")) {
            tty_println("Commands: help, clear, about");
        } else if (streq(line, "clear")) {
            tty_clear(tty_active());
        } else if (streq(line, "about")) {
            tty_println("spiderOS kernel");
        } else if (line[0]) {
            tty_prints("Unknown: ");
            tty_println(line);
        }
    }
}

void shell_key(int key)
{
    tty_t* t = tty_active();
    size_t row;
    tty_get_cursor(t, &row, NULL);

    switch (key) {
        case KEY_LEFT:
            if (g_cur > 0) {
                g_cur--;
                tty_set_cursor(t, row, g_start + (size_t)g_cur);
            }
            break;

        case KEY_RIGHT:
            if (g_cur < g_len) {
                g_cur++;
                tty_set_cursor(t, row, g_start + (size_t)g_cur);
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
