#include "log.h"
#include "tty.h"

static inline tty_t* T(void) {
    return tty_active();
}

void klog(const char* s)
{
    tty_t* t = T();
    if (!t) return;
    while (*s) tty_putc(t, *s++);
}

void klogln(const char* s)
{
    klog(s);
    klog_char('\n');
}

void klog_char(char c)
{
    tty_t* t = T();
    if (!t) return;
    tty_putc(t, c);
}

void klog_dec(uint32_t v)
{
    char buf[11];
    int i = 0;

    if (v == 0) {
        klog_char('0');
        return;
    }

    while (v > 0) {
        buf[i++] = '0' + (v % 10);
        v /= 10;
    }
    while (i--)
        klog_char(buf[i]);
}

void klog_hex32(uint32_t v)
{
    static const char* hex = "0123456789ABCDEF";
    klog("0x");
    for (int i = 28; i >= 0; i -= 4) {
        klog_char(hex[(v >> i) & 0xF]);
    }
}

void klog_hex64(uint64_t v)
{
    static const char* hex = "0123456789ABCDEF";
    klog("0x");
    for (int i = 60; i >= 0; i -= 4) {
        klog_char(hex[(v >> i) & 0xF]);
    }
}
