// kernel/memory.c
#include "memory.h"
#include "multiboot.h"
#include "log.h"
// Символы из linker.ld
extern uint8_t _kernel_start;
extern uint8_t _kernel_end;

void memory_init(uint32_t mb_info_phys)
{
    multiboot_info_t* mbi = (multiboot_info_t*)(uintptr_t)mb_info_phys;

    klogln("=== Memory init ===");
    klog("mbi @ "); klog_hex32(mb_info_phys); klogln("");

    klog("kernel: ");
    klog_hex32((uint32_t)(uintptr_t)&_kernel_start);
    klog(" .. ");
    klog_hex32((uint32_t)(uintptr_t)&_kernel_end);
    klogln("");

    if (mbi->flags & MB_FLAG_MEM) {
        klog("mem_lower KB: "); klog_dec(mbi->mem_lower); klogln("");
        klog("mem_upper KB: "); klog_dec(mbi->mem_upper); klogln("");
    } else {
        klogln("mem_lower/upper: not provided");
    }

    if (!(mbi->flags & MB_FLAG_MMAP)) {
        klogln("mmap: not provided by bootloader");
        return;
    }

    klog("mmap @ "); klog_hex32(mbi->mmap_addr);
    klog(" len "); klog_dec(mbi->mmap_length);
    klogln("");

    uint32_t start = mbi->mmap_addr;
    uint32_t end   = mbi->mmap_addr + mbi->mmap_length;

    int idx = 0;
    for (uint32_t p = start; p < end; ) {
        multiboot_mmap_entry_t* e = (multiboot_mmap_entry_t*)(uintptr_t)p;

        klog("#"); klog_dec((uint32_t)idx++);
        klog(" type "); klog_dec(e->type);
        klog(" addr "); klog_hex64(e->addr);
        klog(" len ");  klog_hex64(e->len);
        klogln("");

        // size + 4 (поле size не включено в size)
        p += e->size + sizeof(e->size);
    }

    klogln("=== Memory init done ===");
}
