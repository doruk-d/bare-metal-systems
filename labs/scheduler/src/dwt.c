#include "dwt.h"

cyc_cnt_t dwt_init(void){
    DEMCR |= (1 << 24); // enables dwt and imt units 

    if ((DWT_CTRL >> 25) & 0x1)
        return UNSUPPORTED_CYCLE_COUNTER;
    
    DWT_CYCCNT = 0; // reset the value of cyccnt to ensure a clean start

    DWT_CTRL |= (1 << 0); // enables cyccnt 
                          
    return SUPPORTED_CYCLE_COUNTER;
}
