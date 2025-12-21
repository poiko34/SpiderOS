#include "syscall.h"
#include "tty.h"
#include "task.h"
#include "log.h"

typedef uint32_t (*sysfn_t)(
    uint32_t, uint32_t, uint32_t, uint32_t, uint32_t
);

static uint32_t sys_putchar(uint32_t c,
                            uint32_t _1,
                            uint32_t _2,
                            uint32_t _3,
                            uint32_t _4)
{
    (void)_1; (void)_2; (void)_3; (void)_4;

    tty_t* t = tty_active();
    if (!t) return (uint32_t)-1;

    tty_putc(t, (char)c);
    return 0;
}

static uint32_t sys_puts(uint32_t ptr,
                         uint32_t _1,
                         uint32_t _2,
                         uint32_t _3,
                         uint32_t _4)
{
    (void)_1; (void)_2; (void)_3; (void)_4;

    tty_t* t = tty_active();
    if (!t) return (uint32_t)-1;

    const char* s = (const char*)ptr;
    if (!s) return (uint32_t)-1;

    tty_write(t, s);
    return 0;
}

static uint32_t sys_exit(uint32_t code,
                         uint32_t _1, uint32_t _2, uint32_t _3, uint32_t _4)
{
    (void)code;
    (void)_1; (void)_2; (void)_3; (void)_4;

    task_t* cur = task_current();

    klog("SYS_EXIT pid=");
    klog_dec(cur->pid);
    klog("\n");

    task_mark_zombie(cur);

    task_switch(task_kernel());

    return 0;
}

static sysfn_t table[] = {
    sys_putchar, // SYS_PUTCHAR = 0
    sys_puts,    // SYS_PUTS    = 1
    sys_exit,    // SYS_EXIT    = 2
};

uint32_t syscall_dispatch(regs_t* r)
{
    uint32_t n = r->eax;

    if (n >= (sizeof(table) / sizeof(table[0])))
        return (uint32_t)-1;

    return table[n](
        r->ebx,
        r->ecx,
        r->edx,
        r->esi,
        r->edi
    );
}
