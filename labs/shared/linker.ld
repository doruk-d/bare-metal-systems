_stack_size = 0x1000;

MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 512K

    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 96K
}

SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector))
        *(.text*)
        . = ALIGN(8);
    
    } > FLASH
    
    .rodata : 
    {
        *(.rodata*)
        . = ALIGN(8);

    } > FLASH

    _sidata = LOADADDR(.data);

    .data : 
    {
        . = ALIGN(8);
        _sdata = . ;
        *(.data*)
        . = ALIGN(8);
        _edata = . ;

    } > RAM AT > FLASH

    .bss :
    {
        . = ALIGN(8);
        _sbss = . ;
        *(.bss*)
        *(COMMON)
        . = ALIGN(8);
        _ebss = . ;
        . = ALIGN(1 << LOG2CEIL(. - _sdata));
        _ebss_mpu = . ;

    } > RAM

    _sheap = _ebss_mpu;

    _stack_start = ORIGIN(RAM) + LENGTH(RAM) - _stack_size;

    .stack _stack_start (NOLOAD) : 
    {
        . = ALIGN(_stack_size);
        _sstack = . ;
        . = . + _stack_size;
        _estack = . ;

    } > RAM

}

ASSERT(_ebss_mpu <= _sstack, "ERROR: MPU BSS Region has collided with stack")
