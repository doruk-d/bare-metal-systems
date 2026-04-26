## Custom Allocator

### Overview

- This project aims to build intuition for low-level memory management and systems programming in C.

### Project Structure
```
    allocator/
    ├── makefile
    ├── docs/
    │   └── README.md
    ├── include/
    │   └── my_malloc.h
    └── src/
        ├── my_malloc_internal.h
        └── my_malloc.c

    tests/
    └── test_allocator/
        └── test_allocator.c
```

**Note**: Uart driver sourced from
`drivers/uart/`

linker script sourced from
`boot/stm32f4x/`



### Development Environment

- The allocator was initially prototyped on x86-64 with the purpose of easier debugging and iteration, before being ported to ARM to target microcontroller environments.
- **Target**: STM32F401RE ARM Cortex-M4

### Allocator Design

- **Strategy**: The allocator implements first-fit allocation with block coalescing through a doubly linked list to reduce fragmentation. 
    - **Trade-off**: Doubly linked list implementation chosen over singly linked list, for the easier coalescing it enables, at the cost of slightly larger metadata allocation.

### Memory Alignment

- As the memory alignment has been a major allocator concern, the allocator enforces architecture-safe alignment rules to prevent misaligned access, data corruption, and undefined behavior via:
    - Inline alignment function that rounds requested sizes to the required alignment boundary,
    - Alignment macro that:
        - Defines the alignment boundary used for rounding allocation sizes
        - Enforces a minimum allocation size to mitigate fragmentation risk
    - Metadata struct encapsulated in a union to guarantee proper header alignment.

### Interrupt Safety

- Allocator operations disable interrupts during critical sections to guarantee atomicity. In interrupt driven environments, this approach provides memory safety by preventing interruption of allocation operations, which could otherwise corrupt the free list and leave the heap in inconsistent state.  

### Heap Lifecycle 

- The allocator uses the function `wipe_heap()` to reinitialize heap metadata and overwrite the memory, preventing information leaks.

### Internal Header

- An internal header exposes the metadata structure to the test driver and source code, without making those part of the public API. 

- This maintains encapsulation while enabling tests to verify the allocator behavior.

### Linker Script Memory Layout

- The linker script places the heap directly after `.bss` section. 
- The stack space is allocated at the top of RAM and grows downward.
- A linker `ASSERT` guarantees that .bss section does not overlap the reserved stack space.

### Stack Safety Instrumentation

- Populated the unallocated stack space with a Stack Canary pattern using the magic number 0xDEADBEEF to detect stack overflow or heap collision.
- Implemented a standard HardFault Handler.

### Build System Decisions

- Employed `vpath` for source discovery alongside `notdir` and `addprefix` to manage object file paths, keeping the Makefile and source directories clean.

### Testing

- Tests include:
    - Validation of stack integrity by walking the stack to the canary boundary and comparing depth against the configured stack size.
    - Verification of allocator behavior under heap exhaustion and full memory reclamation via `wipe_heap()`.
    - Verification of allocator behavior under large continuous allocation and heavy fragmentation.
- Tests run once to prevent results from getting lost in infinite loop output. 

### Bug Discovery

- Uncovered a `size_t` overflow that wrapped the size calculation, undersizing the pointer array and causing out-of-bounds writes during allocation tracking, triggering HardFault.
- Resolved by using a statically sized array with compile-time macros.
- Resolved critical section management issue caused by `cpsid/e i` use. Nested functions with critical sections were re-enabling interrupts mid-critical section on return. Implemented save/restore pattern with PRIMASK, solved the issue by saving and restoring the previous PRIMASK value across function boundaries.

### Memory Layout Verification

- The following output of `arm-none-eabi-readelf` verifies that the linker script produced the correct segment layout. The `PhysAddr/VirtAddr` divergence on segment 01 is what initiates the copy-down at startup.
- In the segment 02,  discrepancy between `FileSiz/Memsiz` indicates the predefined stack size is reserved for runtime stack operations.
```bash
$ arm-none-eabi-readelf -l  build/my_malloc.elf 


Elf file type is EXEC (Executable file)
Entry point 0x8000000
There are 3 program headers, starting at offset 52

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x010000 0x08000000 0x08000000 0x00d78 0x00d78 R E 0x10000
  LOAD           0x020000 0x20000000 0x08000d78 0x00000 0x00160 RW  0x10000
  LOAD           0x007000 0x20017000 0x20017000 0x00000 0x01000 RW  0x10000

 Section to Segment mapping:
  Segment Sections...
   00     .text .rodata 
   01     .data .bss 
   02     .stack 
```
- In the following section, the symbol table ( `.symtab` ) illustrates sections, linker script boundary symbols and global variable addresses aligned in the defined memory map.
- As seen below, the vector table is placed at the base of flash (`08000000`) as required by the Cortex-M boot sequence.
- `_sidata` aligns with the `PhysAddr` of segment 01, confirming the load address of `.data` in flash memory.
- The stack size can be verified through computing _estack - _sstack = 0x20018000 - 0x20017000 = 0x1000 (4KB), aligning with the `_stack_size` symbol.
- MPU alignment can also be seen as `_ebss` sitting at `20000370` whereas `_ebss_mpu` is at `20000400`.
```bash
$ arm-none-eabi-readelf -s  build/my_malloc.elf 

Symbol table '.symtab' contains 115 entries: # relevant entries shown, ... denotes omitted
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     1: 08000000     0 SECTION LOCAL  DEFAULT    1 .text
     2: 08000d88     0 SECTION LOCAL  DEFAULT    2 .rodata
     3: 20000000     0 SECTION LOCAL  DEFAULT    3 .data
     4: 20000000     0 SECTION LOCAL  DEFAULT    4 .bss
     5: 20017000     0 SECTION LOCAL  DEFAULT    5 .stack
     ...
    85: 08001008     0 NOTYPE  GLOBAL DEFAULT  ABS _sidata
    87: 20000400     0 NOTYPE  GLOBAL DEFAULT    4 _ebss_mpu
    88: 20000400     0 NOTYPE  GLOBAL DEFAULT    4 _sheap
    89: 20000000     0 NOTYPE  GLOBAL DEFAULT    4 _sbss
    93: 20000000     0 NOTYPE  GLOBAL DEFAULT    3 _sdata
    97: 20000370     0 NOTYPE  GLOBAL DEFAULT    4 _ebss
   100: 08000000   364 OBJECT  GLOBAL DEFAULT    1 vector_table
   104: 00001000     0 NOTYPE  GLOBAL DEFAULT  ABS _stack_size
   109: 20017000     0 NOTYPE  GLOBAL DEFAULT    5 _sstack
   111: 20018000     0 NOTYPE  GLOBAL DEFAULT    5 _estack
   112: 20000000     0 NOTYPE  GLOBAL DEFAULT    3 _edata
```

- In the following section, `objdump -S` output illustrates the full startup sequence in `ResetHandler`, such as `.data` copy,`.bss` zero and the stack painting with the canary pattern. Before, transfering control to `main()`, which terminates in an infinite loop.
```bash
$ arm-none-eabi-objdump -S build/my_malloc.elf 

build/my_malloc.elf:     file format elf32-littlearm

Disassembly of section .text: # relevant entries shown, ... denotes omitted

08000000 <vector_table>:
 8000000:   00 80 01 20 21 06 00 08 c1 06 00 08 b9 06 00 08
                            .
                            .
08000620 <ResetHandler>:
 ...
    while (s_data < e_data)
 8000648:   e007        b.n 800065a <ResetHandler+0x3a>
        *s_data++ = *f_data++;
 ...
    while (s_bss < e_bss)
 8000662:   e004        b.n 800066e <ResetHandler+0x4e>
        *s_bss++ = 0;
 ...
    while (s_stack < (uint32_t *)current_sp)
 8000676:   e004        b.n 8000682 <ResetHandler+0x62>
        *s_stack++ = 0xDEADBEEF;
 80006b4:   deadbeef    .word 0xdeadbeef
 ...
    main();
 800068e:   f000 f81b   bl  80006c8 <main>
                            .
                            .
080006c8 <main>:
 ...
    while (1);
 80006e8:   bf00        nop
 80006ea:   e7fd        b.n 80006e8 <main+0x20>
 ...

```

### Scope

- As stated, this project is purely educational and is not intended for production use. 
- During the process, developed familiarity with memory layout, fragmentation, atomic sections, and embedded diagnostics as well as experience porting between x86 and ARM architectures.
