#include "mpu_config.h"
#include <stdint.h>

#define MPU_BASE 0xE000ED90
#define MPU_RNR (*(volatile uint32_t *)(MPU_BASE + 0x08))
#define MPU_RASR (*(volatile uint32_t *)(MPU_BASE + 0x10))

extern uint32_t _sstack;

#ifdef NDEBUG
    #define ASSERT(cond) ((void)(cond))
#else
    #define ASSERT(cond) \
        do { \
            if (!(cond)) \
                __asm__ volatile("bkpt #0"); \
        } while (0);
#endif

static void rasr_check(void){
    for (uint8_t i = 0; i < 7; i++){
        MPU_RNR = i;

        switch(i){
            case 0:
                ASSERT(MPU_RASR == 0x07020025);
                break;
            case 1:
                ASSERT(MPU_RASR == 0x1306001F);
                break;
            case 2:
                ASSERT(MPU_RASR == 0x1306001D);
                break;
            // 3 and 4 have base addresses and sizes to be calculated during runtime so their checks can be done with gdb
            case 3:
                break;
            case 4:
                break;
            case 5:
                ASSERT(MPU_RASR == 0x1006000D);
                break;
            case 6:
                ASSERT(MPU_RASR == 0x13050039);
                break;
            default:
                break;
        };
    }        
}

static void mem_fault_trigger(void){
    volatile uint32_t *stack_start = (volatile uint32_t *)&_sstack;
    *stack_start = 0xDEADBEEF;
}

int main(void){
    if (mpu_init() == ERR_MPU_NOT_PRESENT) __asm__ volatile("bkpt #1");

    rasr_check();
    mem_fault_trigger();
    while (1);
}
