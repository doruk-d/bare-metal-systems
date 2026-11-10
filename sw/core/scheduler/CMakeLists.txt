add_library(scheduler STATIC
        src/context_switch.S
        src/dwt.c
        src/measure_gpio.c
        src/scheduler.c
        src/svc.c
        src/systick.c
)

target_include_directories(scheduler PUBLIC include/ ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(scheduler PRIVATE regs)