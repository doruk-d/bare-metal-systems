#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

typedef enum {
    SYS_LNCH = 0,
    SYS_YIELD
}svc_call_t;

typedef enum{
    SUCCESS = 0,
    FAIL
}task_init_t;

task_init_t task_create(void *arg, void (*task_func)(void*));
void yield(void);
void scheduler_init(void);
void scheduler_run(void);
void task_launch(void);

#endif
