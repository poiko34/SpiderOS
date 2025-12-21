#include "task.h"

static task_t kernel_task;
static task_t* g_current = &kernel_task;
static uint32_t g_next_pid = 1;

task_t* task_kernel(void)
{
    return &kernel_task;
}

void task_init_system(void)
{
    kernel_task.pid = 0;
    kernel_task.state = TASK_RUNNING;
    kernel_task.entry = 0;
    kernel_task.user_stack_top = 0;
}

task_t* task_current(void)
{
    return g_current;
}

task_t* task_create_user(uint32_t entry, uint32_t user_stack_top)
{
    static task_t user_task; // пока один user-task

    user_task.pid = g_next_pid++;
    user_task.state = TASK_READY;
    user_task.entry = entry;
    user_task.user_stack_top = user_stack_top;

    return &user_task;
}

void task_mark_zombie(task_t* t)
{
    if (t)
        t->state = TASK_ZOMBIE;
}

void task_switch(task_t* next)
{
    if (!next) return;

    g_current = next;
    next->state = TASK_RUNNING;
}
