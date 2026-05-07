#include "asm_offsets.h"
.extern current_task
.extern next_task

.globl PendSV_Handler
.type PendSV_Handler, %function

PendSV_Handler:
    mrs r0, psp                 @ get the current sp
    stmdb r0!, {r4-r11}         @ push the software frame
    ldr r1, =current_task       @ read current task's address
    ldr r1, [r1]                @ get the address of tcb pointer
    str r0, [r1, #OFFSET_SP]    @ store the new sp into sp pointer

    ldr r0, =next_task          @ get the address of the next task
    ldr r0, [r0]                @ get the tcb pointer address
    ldr r0, [r0, #OFFSET_SP]    @ sp value
    ldmia r0!, {r4-r11}         @ pop the software stack
    msr psp, r0                 @ write the new sp into the psp to correctly pop exception frame

    @ update the current task
    ldr r0, =next_task
    ldr r0, [r0]
    ldr r1, =current_task
    str r0, [r1]

    bx lr                       @ return

.globl SVC_Handler
.type SVC_Handler, %function

SVC_Handler:
    tst lr, #4     
    ite eq         
    mrseq r0, msp  
    mrsne r0, psp  
    bl SVC_Handler_c
    ldr lr, =0xFFFFFFFD
    bx lr
