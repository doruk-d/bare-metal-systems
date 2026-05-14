#include <stdint.h>

#define DEMCR *(volatile uint32_t *)0xE000EDFC
#define DWT_CTRL *(volatile uint32_t *)0xE0001000
#define DWT_CYCCNT *(volatile uint32_t *)0xE0001004 

typedef enum{
    SUPPORTED_CYCLE_COUNTER = 0,
    UNSUPPORTED_CYCLE_COUNTER
}cyc_cnt_t;

cyc_cnt_t dwt_init(void);
