#include "measure_gpio.h"

void gpio_init(void){
    RCC_AHB1ENR |= (1 << 0);

    GPIOA_MODER |= (1 << (PIN * 2));
}

