#include "kbd.h"
#include "io.h"
#include "timer.h"

static const char map[128] = {
  0,27,'1','2','3','4','5','6','7','8','9','0','-','=', '\b',
 '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,
 'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\',
 'z','x','c','v','b','n','m',',','.','/',0,'*',0,' ',
};

int kbd_getkey(void) {

    while (!(inb(0x64) & 1));

    uint8_t sc = inb(0x60);
    static int ext = 0;

    while (!(inb(0x64) & 1));

    if (sc == 0xE0) {
        ext = 1;
        return 0;
    }

    if (sc & 0x80) return 0;

    if (ext) {
        ext = 0;
        switch (sc) {
            case 0x4B: return KEY_LEFT;
            case 0x4D: return KEY_RIGHT;
            case 0x48: return KEY_UP;
            case 0x50: return KEY_DOWN;
        }
    }

    if (sc < 128)
        return map[sc];

    return 0;
}

int kbd_getkey_nb(void) {
    if (!(inb(0x64) & 1))
        return 0;

    uint8_t sc = inb(0x60);

    static int ext = 0;

    if (sc == 0xE0) {
        ext = 1;
        return 0;
    }

    if (sc & 0x80)
        return 0;

    if (ext) {
        ext = 0;
        switch (sc) {
            case 0x4B: return KEY_LEFT;
            case 0x4D: return KEY_RIGHT;
            case 0x48: return KEY_UP;
            case 0x50: return KEY_DOWN;
        }
    }

    if (sc < 128)
        return map[sc];

    return 0;
}
