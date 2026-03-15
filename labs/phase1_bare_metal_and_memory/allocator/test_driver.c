#include "uart.h"
#include "my_malloc.h"
#include "my_malloc_internal.h"
#include <stdint.h>

#define BLOCK_SIZE 4096
#define T_SIZE 32
#define TINY_SIZE (T_SIZE + sizeof(block_header_t))
#define BLOCKS ((BLOCK_SIZE) / (TINY_SIZE))

extern uint32_t _estack;
extern uint32_t _stack_size;
extern const size_t meta_data_size;

void stack_size_check(void);
void test_oom_and_wipe(void);
void test_swiss_cheese(void);

int main(){
    uart_init(9600);
    
    uart_puts("---Allocator Test---\r\n");

    test_oom_and_wipe();        
    test_swiss_cheese();  


    uart_puts("---All Tests Complete. HALTING---\r\n");

    while (1);


}

void stack_size_check(){
    uint32_t *e_stack = &_estack;

    uint32_t count = 0;
    while (*(--e_stack) != 0xDEADBEEF)
        count++;

    size_t size = count * sizeof(uint32_t);

    uart_puts("---stack size: ---\r\n");
    uart_putint(size);
    uart_putc('\r');
    uart_putc('\n');

    if (size == (size_t)&_stack_size)
        uart_puts("---PASS, maximum depth of the stack has been reached---\r\n");
    else if (size > (size_t)&_stack_size)
        uart_puts("---STACK OVERFLOW---\r\n");

}

void test_oom_and_wipe(void){
    uint32_t count_oom = 0;
    void *ptr1 = NULL;
    void *oom_first = NULL;

    while ((ptr1 = my_malloc(8192)) != NULL){
        if (count_oom == 0)
            oom_first = ptr1;
    }
    
    stack_size_check();
    wipe_heap();

    uint32_t count_wipe = 0;
    void *ptr2 = NULL;
    void *wipe_first = NULL;

    while ((ptr2 = my_malloc(8192)) != NULL){
        if (count_wipe == 0)
            wipe_first = ptr2;
    }

    stack_size_check();

    if (count_oom == count_wipe && oom_first == wipe_first)
        uart_puts("---PASS, entire heap utilized and reclaimed with no negative consequences---\r\n");
    else if (count_oom > count_wipe)
        uart_puts("---MEMORY LEAK, wipe_heap() has failed to reclaim memory---\r\n");
    else if (count_oom < count_wipe)
        uart_puts("---MEMORY CORRUPTION, wipe_heap() has corrupted the memory boundaries---\r\n");
    
    wipe_heap();
}

void test_swiss_cheese(void){
    size_t size = (size_t)BLOCK_SIZE;
    void *ptr = my_malloc(size);
    if (!ptr){
        uart_puts("---FAIL, failed to allocate memory for the test swiss cheese---\r\n");
        return;
    }

    my_free(ptr);

    static void *p_arr[BLOCKS];
    
    uint32_t i = 0;
    while (size > (size_t)TINY_SIZE){
        void *fragm = my_malloc((size_t)TINY_SIZE);
        if (!fragm){
            uart_puts("---FAIL, failed to allocate tiny segments in the block---\r\n");
            wipe_heap();
            return;
        }

        *(uint32_t *)fragm = 0xAA;
        p_arr[i++] = fragm;

        size -= (size_t)TINY_SIZE;
    }
    
    for (uint32_t j = 0; j < i; j++){
        if (*(uint32_t *)p_arr[j] != 0xAA){
            uart_puts("---MEMORY LEAK, metadata from one of the blocks overflowed into another---\r\n");
            return;
        }
    }

    uart_puts("--PASS, test_swiss_cheese()---\r\n");

}
