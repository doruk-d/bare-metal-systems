#include "uart.h"
#include <stdint.h>

int main(void){
    if (uart_init(9600) != UART_OK)
        return 0;

    char input_buffer[128];
    while (1){
        // test calls 
        uart_gets(input_buffer, 128);
        uart_puts(input_buffer);
    }

}
