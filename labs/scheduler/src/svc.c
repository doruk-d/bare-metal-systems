#include "scheduler.h"
#include <stdint.h>

#define SCB_ICSR (*(volatile uint32_t *)0xE000ED04)

void SVC_Handler_c(uint32_t *frame){
    uint8_t svc_type = ((uint8_t *)frame[6])[-2]; // in non fault cases pc points to the next instruction

    switch (svc_type){
        case SYS_LNCH:
            task_launch();
            break;
        case SYS_YIELD:
            SCB_ICSR |= (1 << 28);
            break;
    }
}
