# MPU Configuration

- - -
This module configures Cortex-M4 MPU for the STM32F401RE, mapping the memory
protection regions for flash, SRAM, stack and peripherals.

## Memory Layout

```bash
Address     Memory Structure             Linker Symbols
  (High)    +-----------------------+  <-- _estack (Top of RAM)
      │     |                       |
      │     |        .stack         |
      │     |    (Grows Downward)   |
      │     |           │           |
      │     |           ▼           |
      │     +-----------------------+
      │     | ##### STACK GUARD ### |  <-- MPU Protected Region
      │     +-----------------------+  <-- _sstack
      │     |           ▲           |
      │     |           │           |
      │     |         .heap         |
      │     |    (Grows Upward)     |
      │     |                       |
      │     +-----------------------+  <-- _sheap (_ebss_mpu)
      │     | [ MPU Align Padding ] |
      │     +-----------------------+  <-- _ebss
      │     |         .bss          |
      │     +-----------------------+  <-- _sbss / _edata
      │     |         .data         |
  (Low)     +-----------------------+  <-- _sdata (0x20000000)

```

##  Configuration 

- **Critical Section Setup**

Interrupts are disabled via inline assembly instruction `cpsid i` to create an atomic critical section.

- **MPU Presence Check**

Checked the MPU presence by reading `MPU_TYPER`'s `[15:8]` bits, prior to any attempt of configuration. 
If absence is confirmed, function halts the mcu with an inline asm.

- **Region Configuration**

The MPU is first disabled to prevent partial configuration resulting in an unstable or corrupted state.

Per-region configuration sequence starts with an assignment of a region number to the `MPU_RNR` register. Cortex-M4 supports up to 8 regions.

The process continues by defining the base address for a region using the `MPU_RBAR` register. *See: Architecture Decisions.*

With the base address defined, bitfield encoding is performed using a reusable staging variable in two groups, which is later written in a single operation. During bitfield encoding, `mpu_size_val()` helper function used to calculate size values to be encoded. *See: Architecture Decisions.*

Configured Regions:

    - Flash
        - Normal memory, executable, read-only (privileged & unprivileged)
        
    - SRAM (1)
        - Normal memory, non-executable, read-write (privileged & unprivileged)
        
    - SRAM (2) *See: Architecture Decisions.*
        - Normal memory, non-executable, read-write (privileged & unprivileged)

    - Stack Region
        - Normal memory, non-executable, read-write (privileged & unprivileged)
        
    - .data + .bss Region
        - Normal memory, non-executable, read-write (privileged & unprivileged) 
        
    - Stack Guard Region
        - Normal memory, non-executable, no access        

    - Peripheral Address Space
        - Device memory, non-executable, read-write (privileged & unprivileged) 

- **Barriers and Enable**

As a final configuration step, MPU is enabled with `PRIVDEFENA` set, to allow privileged access to default memory map for regions not explicitly covered by the MPU.

After configuration of MPU registers is completed, data synchronization barrier (`DSB`) is implemented to ensure the register writes are committed to memory, followed by an instruction synchronization barrier (`ISB`) to ensure CPU sees the new configuration before fetching subsequent instructions.

- **Critical Section Teardown**

The process is finalized by re-enabling interrupts via `cpsie i`, followed by a success return value.

## Architectural Decisions

MPU base address alignment for the data + bss region relies on hardware convention, since ARM SRAM always starts at a power of two aligned address, which satisfies the MPU requirements.
SRAM is split in two regions to cover the exact available space, avoiding waste of memory.
GDB-driven tests preferred over UART outputs to prevent a possible deadlock scenario.

## BUG: LOG2CEIL(0) Collapse in Empty Test Binary

Encountered a bug where the data + bss MPU region collapsed to zero size in empty test binaries. Due to `LOG2CEIL(0)` alignment expression in linker script.

```
    Remote debugging using :3333
    MemManageHandler () at ../../boot/stm32f4x/fault_handlers.c:5
    5	   __asm__ volatile("bkpt #0");

    (gdb) info reg
    r0             0x1c                28
    r2             0xdeadbeef          -559038737
    r3             0x20017000          536965120
    sp             0x20017fa4          0x20017fa4
    lr             0xfffffff9          -7
    pc             0x8000558           0x8000558 <MemManageHandler+4>

    ; DACCVIOL and MMARVALID both set
    (gdb) p/x ((*(uint32_t *)0xE000ED28) >> 7) & 0x1
    $1 = 0x1
    (gdb) p/x ((*(uint32_t *)0xE000ED28) >> 1) & 0x1
    $2 = 0x1

    ; faulting address matches intentional write target
    (gdb) p/x (*(uint32_t *)0xE000ED34)
    $12 = 0x20017000
    (gdb) p/x (*(uint32_t *)0x20017000)
    $13 = 0xdeadbeef

    ; region dump
    (gdb) p/x (*(uint32_t*)0xE000ED9C) & 0xffffffe0   ; R0 flash
    $4 = 0x8000000
    (gdb) p/x (*(uint32_t*)0xE000EDA0)
    $5 = 0x7020025
    (gdb) p/x (*(uint32_t*)0xE000ED9C) & 0xffffffe0   ; R1 sram 64kb
    $8 = 0x20000000
    (gdb) p/x (*(uint32_t*)0xE000EDA0)
    $9 = 0x1306001f
    (gdb) p/x (*(uint32_t*)0xE000ED9C) & 0xffffffe0   ; R2 sram 32kb
    $10 = 0x20010000
    (gdb) p/x (*(uint32_t*)0xE000EDA0)
    $11 = 0x1306001d
    (gdb) p/x (*(uint32_t*)0xE000ED9C) & 0xffffffe0   ; R3 stack
    $14 = 0x20017000
    (gdb) p/x (*(uint32_t*)0xE000EDA0)
    $15 = 0x13060017
    (gdb) p/x (*(uint32_t*)0xE000ED9C) & 0xffffffe0   ; R4 data+bss - bug: size=32B
    $16 = 0x20000000
    (gdb) p/x (*(uint32_t*)0xE000EDA0)
    $17 = 0x13060009
    (gdb) p/x (*(uint32_t*)0xE000ED9C) & 0xffffffe0   ; R5 stack guard
    $18 = 0x20017000
    (gdb) p/x (*(uint32_t*)0xE000EDA0)
    $19 = 0x1006000d
    (gdb) p/x (*(uint32_t*)0xE000ED9C) & 0xffffffe0   ; R6 peripherals
    $20 = 0x40000000
    (gdb) p/x (*(uint32_t*)0xE000EDA0)
    $21 = 0x13050039

    ; _ebss_mpu collapsed to _sdata - empty test binary, LOG2CEIL(0) BUG!
    (gdb) p/x &_sdata
    $22 = 0x20000000
    (gdb) p/x &_ebss_mpu
    $23 = 0x20000000

    ; after linker fix: . = _sdata + (1 << LOG2CEIL(MAX(. - _sdata, 32)));
    (gdb) p/x &_ebss_mpu
    $2 = 0x20000020
```
