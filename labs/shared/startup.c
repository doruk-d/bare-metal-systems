#include "system_clock.h"
#include "fault_handlers.h"
#include <stdint.h>

extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss_mpu;

void ResetHandler(void);
void DefaultHandler(void);
void USART2_IRQHandler(void) __attribute__((weak, alias("DefaultHandler"))); 

typedef void (*isr_t)(void);

__attribute__((section(".isr_vector")))

const isr_t vector_table[]={
    (isr_t)&_estack,
    ResetHandler,
    [2 ... 97] = DefaultHandler, 
    [3] = HardFaultHandler,
    [4] = MemManageHandler,
    [54] = USART2_IRQHandler
};

void DefaultHandler(void){
    __asm__ volatile("bkpt #0");
}

void ResetHandler(void){
    uint32_t *src_fdata = &_sidata;
    uint32_t *dest_sdata = &_sdata;
    uint32_t *dest_edata = &_edata;
    uint32_t *s_bss = &_sbss;
    uint32_t *e_bss = &_ebss_mpu;

    while (dest_sdata < dest_edata)
        *dest_sdata++ = *src_fdata++;

    while (s_bss < e_bss)
        *s_bss++ = 0;

    system_clock_init();

    extern int main(void);
    main();

}
