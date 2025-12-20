#pragma once
#include <stdint.h>
#include "isr.h"

// Останавливает ядро и печатает диагностику
__attribute__((noreturn))
void panic(const char* msg);

// То же, но с регистровым дампом (удобно вызывать из ISR)
__attribute__((noreturn))
void panic_regs(const char* msg, regs_t* r);

// Утилита: печать file:line + msg
__attribute__((noreturn))
void panic_at(const char* file, int line, const char* msg);

// Макросы
#define PANIC(msg)        panic_at(__FILE__, __LINE__, (msg))
#define PANIC_REGS(msg,r) panic_regs((msg), (r))

// Простенький assert
#define ASSERT(cond) do { if (!(cond)) panic_at(__FILE__, __LINE__, "ASSERT: " #cond); } while (0)
