#ifndef MEASURE_GPIO_H
#define MEASURE_GPIO_H

#include <stdint.h>

#define RCC_AHB1ENR *(volatile uint32_t *)(0x40023800 + 0x30)

#define GPIOA_BASE 0x40020000
#define GPIOA_MODER *(volatile uint32_t *)GPIOA_BASE
#define GPIOA_BSRR  *(volatile uint32_t *)(GPIOA_BASE + 0x18)

#define PIN 9

void gpio_init(void);

#endif
