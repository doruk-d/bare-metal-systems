#include "systick.h"
#include "scheduler.h"
#include <stdint.h>

#define STK_CTRL (*(volatile uint32_t *)0xE000E010)
#define STK_LOAD (*(volatile uint32_t *)0xE000E014)
#define STK_VAL (*(volatile uint32_t *)0xE000E018)

#define CPU_FREQ 84000000
#define SYSTICK_FREQ 1000
#define RELOAD_VAL ((CPU_FREQ / SYSTICK_FREQ) - 1)

void systick_init(void){
    STK_LOAD = RELOAD_VAL;

    STK_VAL = 0;

    STK_CTRL = (1 << 2) | (1 << 1) | (1 << 0);
    
}

void SysTick_Handler(void){
    scheduler_run();
}
