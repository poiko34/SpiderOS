#include "tty.h"
#include "shell.h"

static tty_t* g_active = 0;

void tty_init(tty_t* t, struct tty_ops ops, size_t cols, size_t rows)
{
    t->ops  = ops;
    t->cols = cols;
    t->rows = rows;
}

tty_t* tty_active(void)
{
    return g_active;
}

void tty_set_active(tty_t* t)
{
    g_active = t;
}

void tty_input_char(char c) { shell_input(c); }
void tty_input_key(int key) { shell_key(key); }
void tty_write(tty_t* t, const char* s) { while (*s) tty_putc(t, *s++); }
void tty_writeln(tty_t* t, const char* s) { tty_write(t, s); tty_putc(t, '\n'); }