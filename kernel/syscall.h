#pragma once
#include <stdint.h>
#include "isr.h"

#define SYS_PUTCHAR  0
#define SYS_PUTS     1
#define SYS_EXIT     2


uint32_t syscall_dispatch(regs_t* r);
