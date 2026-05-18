.syntax unified
.thumb

#include "asm_offsets.h"
#define DWT_CYCCNT_ADDR 0xE0001004
#define GPIOA_BSRR_ADDR 0x40020018

.globl PendSV_Handler
.type PendSV_Handler, %function

PendSV_Handler:
    ldr r0, =GPIOA_BSRR_ADDR
    ldr r1, =0x02000000
    str r1, [r0]

    ldr r2, =DWT_CYCCNT_ADDR     
    ldr r2, [r2]                @ read the current value of cyccnt
    ldr r3, =sw_start_cyccnt
    str r2, [r3]

    ldr r2, =current_task
    ldr r2,[r2]
    cbz r2, skip_save
    mrs r0, psp                 @ get the current sp
    stmdb r0!, {r4-r11}         @ push the software frame
    ldr r1, =current_task       @ read current task's address
    ldr r1, [r1]                @ get the address of tcb pointer
    str r0, [r1, #OFFSET_SP]    @ store the new sp into sp pointer

    skip_save:
    ldr r0, =next_task          @ get the address of the next task
    ldr r0, [r0]                @ get the tcb pointer address
    ldr r0, [r0, #OFFSET_SP]    @ sp value
    ldmia r0!, {r4-r11}         @ pop the software stack
    msr psp, r0                 @ write the new sp into the psp to correctly pop exception frame

    ldr r2, =current_task
    ldr r2, [r2]
    cbnz r2, not_boot
    push {lr}
    bl systick_init
    pop {lr}
    ldr lr, =0xfffffffd

    not_boot:
    @ update the current task
    ldr r0, =next_task
    ldr r0, [r0]
    ldr r1, =current_task
    str r0, [r1]

    ldr r3, =DWT_CYCCNT_ADDR     
    ldr r3, [r3]                @ read the current value of cyccnt
    ldr r2, =sw_end_cyccnt
    str r3, [r2]

    bx lr                       @ return

.globl SVC_Handler
.type SVC_Handler, %function

SVC_Handler:
    tst lr, #4     
    ite eq         
    mrseq r0, msp  
    mrsne r0, psp  
    push {lr}
    bl SVC_Handler_c
    pop {pc}
