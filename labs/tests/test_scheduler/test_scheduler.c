#include <stdint.h>
#include <stddef.h>
#include "dwt.h"
#include "uart.h"
#include "scheduler.h"

void task1(void*);
void task2(void*);
void task3(void*);

int main(void){
    uart_init(9600);

    task_create(NULL,&task1); 
    task_create(NULL,&task2); 
    task_create(NULL,&task3);

    dwt_init();
    scheduler_init();

}

void task1(void*){
    uart_puts("TASK_1 running..."); 
    while(1);
}

void task2(void*){
    uart_puts("TASK_2 running..."); 
    while(1);
}

void task3(void*){
    uart_puts("TASK_3 running..."); 
    while(1);
}
