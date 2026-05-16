#include "scheduler.h"
#include <stdint.h>

void SVC_Handler_c(uint32_t *frame){
    uint8_t svc_type = ((uint8_t *)frame[6])[-2]; // in non fault cases pc points to the next instruction

    switch (svc_type){
        case SYS_LNCH:
            task_launch();
            break;
        case SYS_YIELD:
            scheduler_run();
            break;
    }
}
