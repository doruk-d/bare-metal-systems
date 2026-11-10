add_library(bsp STATIC
        fault_handlers.c
        mpu_config.c
        system_clock.c
)

target_link_libraries(bsp PUBLIC regs)

target_include_directories(bsp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
