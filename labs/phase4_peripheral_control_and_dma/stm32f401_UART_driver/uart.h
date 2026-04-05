#ifndef UART_H
#define UART_H

#include <stdint.h>

typedef enum{
    UART_OK,
    UART_ERR_INVALID_BAUD
}uart_err_t;

uart_err_t uart_init(uint32_t baud);
void USART2_IRQHandler(void);
void uart_putc(char c);
void uart_puts(char *s);
uint8_t uart_getc(void);
void uart_gets(char *s, uint32_t max_len);
void uart_putint(uint32_t value);

#endif
