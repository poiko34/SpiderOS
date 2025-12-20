#include "kbd.h"
#include "io.h"
#include "timer.h"
#include "shell.h"
#include "isr.h"
#include "tty.h"

static int shift = 0;
static int caps  = 0;
static int ext   = 0;
static int ctrl = 0;

static const char keymap[128] = {
    0,  27,'1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'','`', 0,'\\',
    'z','x','c','v','b','n','m',',','.','/', 0, '*', 0,' ',
    /* остальное можно оставить 0 */
};

static const char keymap_shift[128] = {
    0,  27,'!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n', 0,
    'A','S','D','F','G','H','J','K','L',':','"','~', 0,'|',
    'Z','X','C','V','B','N','M','<','>','?', 0, '*', 0,' ',
};

static void keyboard_cb(regs_t* r)
{
    (void)r;
    uint8_t sc = inb(0x60);
    kbd_handle_scancode(sc);
}

void kbd_install(void)
{
    register_interrupt_handler(33, keyboard_cb);
}

void kbd_handle_scancode(uint8_t sc)
{
    // extended prefix
    if (sc == 0xE0) {
        ext = 1;
        return;
    }

    int released = sc & 0x80;
    sc &= 0x7F;

    // ----- EXTENDED KEYS (стрелки) -----
    if (ext) {
        ext = 0;
        if (released) return;

        switch (sc) {
            case 0x4B: shell_key(KEY_LEFT);  return;
            case 0x4D: shell_key(KEY_RIGHT); return;
            case 0x48: shell_key(KEY_UP);    return;
            case 0x50: shell_key(KEY_DOWN);  return;
        }
        return;
    }

    // Shift
    if (sc == 0x2A || sc == 0x36) {
        shift = !released;
        return;
    }

    // CapsLock
    if (sc == 0x3A && !released) {
        caps = !caps;
        return;
    }

    // Ctrl
    if (sc == 0x1D) {
        ctrl = !released;
        return;
    }

    if (released)
        return;

    char c = shift ? keymap_shift[sc] : keymap[sc];

    // Caps affects letters
    if (caps && c >= 'a' && c <= 'z')
        c -= 32;
    else if (caps && c >= 'A' && c <= 'Z' && shift)
        c += 32;

    // Ctrl+A..Z
    if (ctrl && c >= 'a' && c <= 'z')
        c = c - 'a' + 1;

    if (c)
        shell_input(c);
}