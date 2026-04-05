#include "uart.h"
#include <stdint.h>

#define PCLK1_FREQ 42000000UL
#define GPIOA_BASE 0x40020000
#define USART_BASE 0x40004400
#define RCC_BASE 0x40023800
#define STK_BASE 0xE000E010
#define NVIC_ISER_BASE 0xE000E100

#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40))

#define GPIOA_MODER (*(volatile uint32_t *)GPIOA_BASE)
#define GPIOA_OSPEEDR (*(volatile uint32_t *)(GPIOA_BASE + 0x08))

#define USART_SR (*(volatile uint32_t *)USART_BASE)
#define USART_DR (*(volatile uint32_t *)(USART_BASE + 0x04))
#define USART_BRR (*(volatile uint32_t *)(USART_BASE + 0x08))
#define USART_CR1 (*(volatile uint32_t *)(USART_BASE + 0x0C))

#define NUM_IRQ 38
#define NVIC_BIT (NUM_IRQ % 32)
#define NVIC_ISERx (*(volatile uint32_t *)(NVIC_ISER_BASE + (0x04 * (NUM_IRQ / 32))))

#define PIN 2
#define PIN2 3
#define RX_BUFF_SIZE 256
#define TX_BUFF_SIZE 256

static volatile uint8_t rx_buff[RX_BUFF_SIZE];
static volatile uint32_t head_rx = 0;
static volatile uint32_t tail_rx = 0;

static volatile uint8_t tx_buff[TX_BUFF_SIZE];
static volatile uint32_t head_tx = 0;
static volatile uint32_t tail_tx = 0;

static inline uint32_t rx_available(void){
    return head_rx != tail_rx;
}

static inline uint32_t tx_available(void){
    return head_tx != tail_tx;
}

static inline void gpio_set_af(uint32_t pin){
    GPIOA_MODER &= ~(3 << (pin * 2));
    GPIOA_MODER |= (2 << (pin * 2));

    GPIOA_OSPEEDR |= (1 << (pin * 2));

    uint32_t offset = (pin < 8) ? 0x20 : 0x24;
    uint32_t shift = (pin % 8) * 4;
    
    volatile uint32_t *reg_AF = (volatile uint32_t *)(GPIOA_BASE + offset);
    
    *reg_AF &= ~(15 << shift);
    *reg_AF |= (7 << shift);
}

uart_err_t uart_init(uint32_t baud){
    if (baud == 0 || baud > 2625000) 
        return UART_ERR_INVALID_BAUD; 

    // set the GPIOA clock on 
    RCC_AHB1ENR |= (1 << 0);

    // set the USART2 clock on 
    RCC_APB1ENR |= (1 << 17);

    // GPIOA settings
    gpio_set_af(PIN);
    gpio_set_af(PIN2);

    // set AF 
    // set baud rate
    USART_BRR = (PCLK1_FREQ + (baud / 2)) / baud;

    // set TE, RE on
    USART_CR1 |= (1 << 2) | (1 << 3);

    // UE enable 
    USART_CR1 |= (1 << 13);

    // RXNEIE enable
    USART_CR1 |= (1 << 5);

    // NVIC enable
    NVIC_ISERx |= (1 << NVIC_BIT);

    // enable interrupts globally
    __asm__ volatile("cpsie i" ::: "memory");

    return UART_OK;
}

void USART2_IRQHandler(void){
    if (((USART_SR >> 5) & 0x1)){ 
        uint8_t c = USART_DR; // read first to clear the RXNE flag so interrupt does not fire again

        if (((head_rx + 1) % RX_BUFF_SIZE == tail_rx))
            return;

        rx_buff[head_rx] = c;
        
        head_rx = (head_rx + 1) % RX_BUFF_SIZE;
    }

    // check if TXE is high, which is the main reason of tx interrupt
    if (((USART_SR >> 7) & 0x1)){
        if (tx_available()){
            USART_DR = tx_buff[tail_tx];

            tail_tx = (tail_tx + 1) % TX_BUFF_SIZE;
        } 
        else {
            // clear the interrupt flag if buffer is empty 
            USART_CR1 &= ~(1 << 7);
        }
    }
}

void uart_putc(char c){
    // check if buffer is full, if so drop the byte
    if ((head_tx + 1) % TX_BUFF_SIZE == tail_tx)
        return;

    // store the byte
    tx_buff[head_tx] = c;
    
    head_tx = (head_tx + 1) % TX_BUFF_SIZE;

    // enable the interrupt
    USART_CR1 |= (1 << 7);
}

void uart_puts(char *s){
    while (*s)
        uart_putc(*s++);
}

uint8_t uart_getc(void){
    // wait if RXNE has data to read
    while (!rx_available());

    uint8_t c = rx_buff[tail_rx];
    tail_rx = (tail_rx + 1) % RX_BUFF_SIZE;
    return c;

}
void uart_gets(char *s, uint32_t max_len){
    uint32_t i = 0;
    while (i < max_len - 1){
        uint8_t ch = uart_getc();

        uart_putc(ch);

        if (ch == '\r' || ch == '\n'){
            uart_putc('\r');
            uart_putc('\n');
            break;
        }
        
        s[i++] = ch;
    }

    s[i] = '\0';
}

void uart_putint(uint32_t value){
    char buff[10]; 

    int8_t i = 0;

    do{
        uint32_t new_val = value % 10;
        uint32_t ch = new_val + '0';
        buff[i++] = ch;
        value /= 10;
    } while (value);

    for (int8_t j = i - 1; j >= 0; j--)
        uart_putc(buff[j]);
}
