#pragma once

#define KEY_LEFT   0x100
#define KEY_RIGHT  0x101
#define KEY_UP     0x102
#define KEY_DOWN   0x103

int kbd_getkey(void);
int kbd_getkey_nb(void);
