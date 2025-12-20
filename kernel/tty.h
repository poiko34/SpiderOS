#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct tty tty_t;

typedef void (*tty_write_char_fn)(char c);
typedef void (*tty_set_cursor_fn)(size_t row, size_t col);
typedef void (*tty_get_cursor_fn)(size_t* row, size_t* col);
typedef void (*tty_cursor_show_fn)(void);
typedef void (*tty_cursor_hide_fn)(void);
typedef void (*tty_cursor_blink_fn)(void);
typedef void (*tty_put_at_fn)(char c, size_t row, size_t col);
typedef void (*tty_clear_fn)(void);

struct tty_ops {
    tty_write_char_fn   write_char;
    tty_put_at_fn       put_at;
    tty_clear_fn        clear;

    tty_set_cursor_fn   set_cursor;
    tty_get_cursor_fn   get_cursor;

    tty_cursor_show_fn  cursor_show;
    tty_cursor_hide_fn  cursor_hide;
    tty_cursor_blink_fn cursor_blink;
};

struct tty {
    struct tty_ops ops;
    size_t cols;
    size_t rows;
};

void tty_init(tty_t* t, struct tty_ops ops, size_t cols, size_t rows);

// Удобные обёртки
static inline void tty_putc(tty_t* t, char c)              { t->ops.write_char(c); }
static inline void tty_put_at(tty_t* t, char c, size_t r, size_t c2) { t->ops.put_at(c, r, c2); }
static inline void tty_clear(tty_t* t)                    { t->ops.clear(); }
static inline void tty_set_cursor(tty_t* t, size_t r, size_t c)      { t->ops.set_cursor(r, c); }
static inline void tty_get_cursor(tty_t* t, size_t* r, size_t* c)    { t->ops.get_cursor(r, c); }
static inline void tty_cursor_show(tty_t* t)              { t->ops.cursor_show(); }
static inline void tty_cursor_hide(tty_t* t)              { t->ops.cursor_hide(); }
static inline void tty_cursor_blink(tty_t* t)             { t->ops.cursor_blink(); }

// Глобальный активный tty (пока один)
tty_t* tty_active(void);
void tty_set_active(tty_t* t);
void tty_input_char(char c);
void tty_input_key(int key);
void tty_write(tty_t* t, const char* s);
void tty_writeln(tty_t* t, const char* s);