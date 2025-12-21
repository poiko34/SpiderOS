#include <stdint.h>
#include <stddef.h>

void* memset(void* dst, int val, size_t count)
{
    uint8_t* p = (uint8_t*)dst;
    while (count--) {
        *p++ = (uint8_t)val;
    }
    return dst;
}
