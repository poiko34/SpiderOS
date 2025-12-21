#pragma once
#include <stdint.h>

typedef enum {
    TASK_UNUSED = 0,
    TASK_READY,
    TASK_RUNNING,
    TASK_ZOMBIE
} task_state_t;

typedef struct task {
    uint32_t pid;
    task_state_t state;

    // user entry + stack (пока без сохранения полного контекста)
    uint32_t entry;
    uint32_t user_stack_top;
} task_t;

void task_init_system(void);
task_t* task_create_user(uint32_t entry, uint32_t user_stack_top);
task_t* task_current(void);
void task_mark_zombie(task_t* t);
void task_switch(task_t* next);
task_t* task_kernel(void);