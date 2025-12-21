#pragma once
#include <stdint.h>

#define USER_STACK_TOP 0x00F00000

void enter_user_mode(uint32_t entry, uint32_t user_stack);
