#include "kbd.h"
#include "io.h"
#include "timer.h"
#include "shell.h"
#include "isr.h"
#include "tty.h"

static int shift = 0;
static int caps  = 0;
static int ext   = 0;

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

    // Shift
    if (sc == 0x2A || sc == 0x36) {
        shift = !released;
        return;
    }

    // CapsLock (toggle on press)
    if (sc == 0x3A && !released) {
        caps = !caps;
        return;
    }

    // ----- EXTENDED KEYS (стрелки) -----
    if (ext) {
        ext = 0;
        if (released) return;

        switch (sc) {
            case 0x4B: tty_input_key(KEY_LEFT);  return; // ←
            case 0x4D: tty_input_key(KEY_RIGHT); return; // →
            case 0x48: tty_input_key(KEY_UP);    return; // ↑
            case 0x50: tty_input_key(KEY_DOWN);  return; // ↓
        }
        return;
    }

    if (released) return;

    char c;
    if (shift)
        c = keymap_shift[sc];
    else
        c = keymap[sc];

    // Caps affects letters only
    if (caps && c >= 'a' && c <= 'z')
        c -= 32;
    else if (caps && c >= 'A' && c <= 'Z' && shift)
        c += 32;

    if (c)
        tty_input_char(c);
}







// int kbd_getkey(void) {

//     while (!(inb(0x64) & 1));

//     uint8_t sc = inb(0x60);
//     static int ext = 0;

//     while (!(inb(0x64) & 1));

//     if (sc == 0xE0) {
//         ext = 1;
//         return 0;
//     }

//     if (sc & 0x80) return 0;

//     if (ext) {
//         ext = 0;
//         switch (sc) {
//             case 0x4B: return KEY_LEFT;
//             case 0x4D: return KEY_RIGHT;
//             case 0x48: return KEY_UP;
//             case 0x50: return KEY_DOWN;
//         }
//     }

//     if (sc < 128)
//         return keymap[sc];

//     return 0;
// }

// int kbd_getkey_nb(void) {
//     if (!(inb(0x64) & 1))
//         return 0;

//     uint8_t sc = inb(0x60);

//     static int ext = 0;

//     if (sc == 0xE0) {
//         ext = 1;
//         return 0;
//     }

//     if (sc & 0x80)
//         return 0;

//     if (ext) {
//         ext = 0;
//         switch (sc) {
//             case 0x4B: return KEY_LEFT;
//             case 0x4D: return KEY_RIGHT;
//             case 0x48: return KEY_UP;
//             case 0x50: return KEY_DOWN;
//         }
//     }

//     if (sc < 128)
//         return keymap[sc];

//     return 0;
// }
