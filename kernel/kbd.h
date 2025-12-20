#pragma once
#include <stdint.h>

#define KEY_LEFT   0x81
#define KEY_RIGHT  0x82
#define KEY_UP     0x83
#define KEY_DOWN   0x84

int kbd_getkey(void);
int kbd_getkey_nb(void);
void kbd_install(void);
void kbd_handle_scancode(uint8_t sc);