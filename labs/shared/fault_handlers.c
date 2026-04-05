#include "fault_handlers.h"

void MemManageHandler(void){
    __asm__ volatile("bkpt #0");
}

// TBD: after gdb session, implement `naked` + stack frame extraction
void HardFaultHandler(void){
    __asm__ volatile("bkpt #1");
}

void DefaultHandler(void){
    __asm__ volatile("bkpt #2");
}
