#include "mpu_config.h"
#include <stdint.h>

#define FLASH_BASE 0x08000000
#define SRAM_BASE 0x20000000
#define MPU_BASE 0xE000ED90
#define PERIPHERAL_BASE 0x40000000

#define MPU_TYPER (*(volatile uint32_t *)MPU_BASE)
#define MPU_CTRL (*(volatile uint32_t *)(MPU_BASE + 0x04))
#define MPU_RNR (*(volatile uint32_t *)(MPU_BASE + 0x08))
#define MPU_RBAR (*(volatile uint32_t *)(MPU_BASE + 0x0C))
#define MPU_RASR (*(volatile uint32_t *)(MPU_BASE + 0x10))
#define SHCSR (*(volatile uint32_t *)0xE000ED24)

#define FLASH_SIZE (512 * 1024U)
#define STACK_G_SIZE 128 // an average number to catch possible overflows
#define PERIPHERAL_SIZE (512 * 1024U * 1024U)

extern uint32_t _sdata;
extern uint32_t _ebss_mpu;
extern uint32_t _estack;
extern uint32_t _sstack;

static inline uint32_t mpu_size_val(uint32_t size){
    if (size <= 32)
        return 4;
    return (32 - __builtin_clz(size - 1)) - 1;
}

mpu_err_t mpu_init(void){
    // to avoid unexpected behavior disable the interrupts
    __asm__ volatile("cpsid i" ::: "memory");

    // check if mpu is present
    if (((MPU_TYPER >> 8) & 0xFF) == 0x00)
        return ERR_MPU_NOT_PRESENT;

    // disable mpu and its features for now
    MPU_CTRL = 0;


    // 0: flash
    MPU_RNR = 0;

    // set base address
    MPU_RBAR = FLASH_BASE;

    uint32_t val = 0;
    // set TEX, S, C, B according to the datasheet
    val |= (0b000 << 19) | (0 << 18) | (1 << 17) | (0 << 16);
    // set XN, AP, SIZE, ENABLE
    val |= (0 << 28) | (0b111 << 24) | (mpu_size_val(FLASH_SIZE) << 1) | (1 << 0);

    MPU_RASR = val;
    val = 0;


    // 1: SRAM size is not a power of 2 
    // split it into two regions to match the physical size 
    MPU_RNR = 1;

    MPU_RBAR = SRAM_BASE;

    val |= (0b000 << 19) | (1 << 18) | (1 << 17) | (0 << 16); 
    val |= (1 << 28) | (0b011 << 24) | (mpu_size_val(64 * 1024U) << 1) | (1 << 0);

    MPU_RASR = val;
    val = 0;

    // 2: RAM second region remaining 32KB
    MPU_RNR = 2;

    MPU_RBAR = SRAM_BASE + (64 * 1024U);

    val |= (0b000 << 19) | (1 << 18) | (1 << 17) | (0 << 16); 
    val |= (1 << 28) | (0b011 << 24) | (mpu_size_val(32 * 1024U) << 1) | (1 << 0);

    MPU_RASR = val;
    val = 0;


    // 3: stack
    MPU_RNR = 3;

    uint32_t size_stack = (uint32_t)&_estack - (uint32_t)&_sstack;

    MPU_RBAR = (uint32_t)&_sstack;

    val |= (0b000 << 19) | (1 << 18) | (1 << 17) | (0 << 16);
    val |= (1 << 28) | (0b011 << 24) | (mpu_size_val(size_stack) << 1) | (1 << 0);

    MPU_RASR = val;
    val = 0;


    // 4: .data + .bss section
    MPU_RNR = 4;

    uint32_t size_data_bss = (uint32_t)&_ebss_mpu - (uint32_t)&_sdata;

    MPU_RBAR = (uint32_t)&_sdata;

    val |= (0b000 << 19) | (1 << 18) | (1 << 17) | (0 << 16);
    val |= (1 << 28) | (0b011 << 24) | (mpu_size_val(size_data_bss) << 1) | (1 << 0);

    MPU_RASR = val;
    val = 0;


    // 5: stack guard  to detect collision between .stack and .data/.bss
    // triggers a fault on overflow from either side 
    MPU_RNR = 5;

    MPU_RBAR = (uint32_t)&_sstack;

    val |= (0b000 << 19) | (1 << 18) | (1 << 17) | (0 << 16);
    val |= (1 << 28) | (0b000 << 24) | (mpu_size_val(STACK_G_SIZE) << 1) | (1 << 0);

    MPU_RASR = val;
    val = 0;


    // 6: peripherals 
    MPU_RNR = 6;

    MPU_RBAR = PERIPHERAL_BASE;

    // configure as device
    val |= (0b000 << 19) | (1 << 18) | (0 << 17) | (1 << 16);
    val |= (1 << 28) | (0b011 << 24) | (mpu_size_val(PERIPHERAL_SIZE) << 1) | (1 << 0);

    MPU_RASR = val;
    val = 0;

    // if buffer delay is a concern add strongly ordered configurations here
    
    // enable PRIVDEFENA for fallback in case mpu mapping is not enough and enable mpu
    MPU_CTRL |= (1 << 2) | (1 << 0);

    // ensure all data synchronization is complete
    __asm__ volatile("dsb" ::: "memory");
    // flush cpu pipeline and discard already fetched then re-fetch from memory
    __asm__ volatile("isb" ::: "memory");

    __asm__ volatile("cpsie i" ::: "memory");

    // enable memory management fault
    SHCSR |= (1 << 16);

    return MPU_CONFIG_SUCCESS;
}


