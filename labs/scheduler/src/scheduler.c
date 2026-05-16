#include "scheduler.h"
#include "systick.h"
#include "asm_offsets.h"
#include "dwt.h"
#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>

#define SCB_ICSR (*(volatile uint32_t *)0xE000ED04)
#define SCB_SHPR3 (*(volatile uint32_t *)0xE000ED20)

#define MAX_TASKS 8

typedef struct task{
    uint32_t *sp;
    uint32_t stack[256] __attribute__((aligned(alignof(max_align_t)))); // ensure the stack alignment
    struct task *next;
}task_t;

static task_t *head = NULL;
static task_t *tail = NULL;

static task_t task_array[MAX_TASKS];
static uint8_t count_of_tasks = 0;

static void scheduler_register(task_t *task);
static void task_remove(void);


task_t *current_task = NULL;
task_t *next_task = NULL;

volatile uint32_t excp_start_cyccnt, sw_start_cyccnt, sw_end_cyccnt;

task_init_t task_create(void *arg, void (*task_func)(void *)){
    if (task_func == NULL)
        return FAIL;

    // if limit of maximum tasks reached it should just skip
    if (count_of_tasks == MAX_TASKS)
        return FAIL;

    task_t *task = &task_array[count_of_tasks++];

    uint32_t *sp = task->stack + 256;

    *--sp = 0x01000000; // thumb bit set for xPSR
    *--sp = (uint32_t)task_func; // pc
    *--sp = (uint32_t)task_remove; // a task should not return if returns it should be removed
    *--sp = 0; // r12
    *--sp = 0; // r3
    *--sp = 0; // r2
    *--sp = 0; // r1
    *--sp = (uint32_t)arg; // r0

    // r4-r11
    *--sp = 0; // r11
    *--sp = 0; 
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0; // r4
            
    task->sp = sp;

    scheduler_register(task);

    return SUCCESS;
}

void yield(void){
    __asm__ volatile("svc %0" ::"i"(SYS_YIELD));
}

static void scheduler_register(task_t *task){
    task->next = NULL;

    if (head == NULL)
        head = tail = task;
    else{
        tail->next = task;
        tail = tail->next;
    }

    tail->next = head;
}

void scheduler_init(void){
    // set pendsv priority to the least
    SCB_SHPR3 |= (0xFF << 16);
    
    // if the linked list empty mcu should halt
    if (head == NULL){
        while(1)
            __asm__ volatile("wfi");
    }

    current_task = NULL;
    next_task = head;    

    SCB_ICSR |= (1 << 28);
}

void scheduler_run(void){
    next_task = current_task->next;
    
    SCB_ICSR |= (1 << 28);
}

static void task_remove(void){
    __asm__ volatile("cpsid i");

    // store the task to remove
    task_t *task_to_remove = current_task;

    // traverse the list and remove it
    if (task_to_remove->next == task_to_remove){
        head = tail = NULL;
        // if all the tasks are removed mcu should be in low power mode
        while(1)
            __asm__ volatile("wfi");
    }
    else{
        task_t *r_task = head;

        while (r_task->next != task_to_remove)
            r_task = r_task->next;

        r_task->next = r_task->next->next;

        if (head == task_to_remove)
            head = task_to_remove->next;

        if (tail == task_to_remove)
            tail = r_task;
    }

    // set the next task to make sure it points to the correct next task
    // in case of systick not fired until the death of the task
    next_task = current_task->next;

    // make sure every read/write operation is complete before pending PENDSV
    __asm__ volatile("dsb");
    
    // trigger a task switch
    SCB_ICSR |= (1 << 28);

    __asm__ volatile("cpsie i");

    while(1);

}
