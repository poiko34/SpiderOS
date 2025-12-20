#pragma once
#include <stdint.h>

void klog(const char* s);
void klogln(const char* s);

void klog_char(char c);
void klog_dec(uint32_t v);
void klog_hex32(uint32_t v);
void klog_hex64(uint64_t v);