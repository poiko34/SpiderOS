// kernel/memory.c
#include "memory.h"
#include "multiboot.h"
#include "vga.h"

// Символы из linker.ld
extern uint8_t _kernel_start;
extern uint8_t _kernel_end;

static void vga_print_hex64(uint64_t v) {
    const char* hex = "0123456789ABCDEF";
    vga_print("0x");
    for (int i = 15; i >= 0; --i) {
        uint8_t nib = (uint8_t)((v >> ((uint64_t)i * 4)) & 0xFULL);
        vga_putc(hex[nib]);
    }
}

void memory_init(uint32_t mb_info_phys)
{
    multiboot_info_t* mbi = (multiboot_info_t*)(uintptr_t)mb_info_phys;

    vga_println("=== Memory init ===");
    vga_print("mbi @ "); vga_print_hex32(mb_info_phys); vga_putc('\n');

    vga_print("kernel: ");
    vga_print_hex32((uint32_t)(uintptr_t)&_kernel_start);
    vga_print(" .. ");
    vga_print_hex32((uint32_t)(uintptr_t)&_kernel_end);
    vga_putc('\n');

    if (mbi->flags & MB_FLAG_MEM) {
        vga_print("mem_lower KB: "); vga_print_dec(mbi->mem_lower); vga_putc('\n');
        vga_print("mem_upper KB: "); vga_print_dec(mbi->mem_upper); vga_putc('\n');
    } else {
        vga_println("mem_lower/upper: not provided");
    }

    if (!(mbi->flags & MB_FLAG_MMAP)) {
        vga_println("mmap: not provided by bootloader");
        return;
    }

    vga_print("mmap @ "); vga_print_hex32(mbi->mmap_addr);
    vga_print(" len "); vga_print_dec(mbi->mmap_length);
    vga_putc('\n');

    uint32_t start = mbi->mmap_addr;
    uint32_t end   = mbi->mmap_addr + mbi->mmap_length;

    int idx = 0;
    for (uint32_t p = start; p < end; ) {
        multiboot_mmap_entry_t* e = (multiboot_mmap_entry_t*)(uintptr_t)p;

        vga_print("#"); vga_print_dec((uint32_t)idx++);
        vga_print(" type "); vga_print_dec(e->type);
        vga_print(" addr "); vga_print_hex64(e->addr);
        vga_print(" len ");  vga_print_hex64(e->len);
        vga_putc('\n');

        // size + 4 (поле size не включено в size)
        p += e->size + sizeof(e->size);
    }

    vga_println("=== Memory init done ===");
}
