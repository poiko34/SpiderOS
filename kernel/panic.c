#include "panic.h"
#include "log.h"
#include "tty.h"

static void dump_regs(regs_t* r)
{
    klog("EAX="); klog_hex32(r->eax); klog(" EBX="); klog_hex32(r->ebx);
    klog(" ECX="); klog_hex32(r->ecx); klog(" EDX="); klog_hex32(r->edx); klogln("");

    klog("ESI="); klog_hex32(r->esi); klog(" EDI="); klog_hex32(r->edi);
    klog(" EBP="); klog_hex32(r->ebp); klog(" ESP="); klog_hex32(r->esp); klogln("");

    klog("EIP="); klog_hex32(r->eip); klog(" CS="); klog_hex32(r->cs);
    klog(" EFL="); klog_hex32(r->eflags); klogln("");

    klog("INT="); klog_dec(r->int_no);
    klog(" ERR="); klog_hex32(r->err_code);
    klogln("");
}

__attribute__((noreturn))
static void halt_forever(void)
{
    // отключаем прерывания и уходим в hlt
    __asm__ volatile("cli");
    for (;;) __asm__ volatile("hlt");
}

void panic(const char* msg)
{
    // можно дополнительно: tty_setcolor, но пока без цветов
    klogln("");
    klogln("========== KERNEL PANIC ==========");
    if (msg) klogln(msg);
    klogln("==================================");
    halt_forever();
}

void panic_at(const char* file, int line, const char* msg)
{
    klogln("");
    klogln("========== KERNEL PANIC ==========");
    klog("at "); klog(file); klog(":"); klog_dec((uint32_t)line); klogln("");
    if (msg) klogln(msg);
    klogln("==================================");
    halt_forever();
}

void panic_regs(const char* msg, regs_t* r)
{
    klogln("");
    klogln("========== KERNEL PANIC ==========");
    if (msg) klogln(msg);
    if (r) dump_regs(r);
    klogln("==================================");
    halt_forever();
}
