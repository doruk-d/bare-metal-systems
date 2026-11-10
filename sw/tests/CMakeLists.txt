add_executable(test_allocator
        test_allocator/test_allocator.c
        test_allocator/test_startup.c
)

target_link_libraries(test_allocator
        PRIVATE
        uart
        allocator
        bsp
)

target_include_directories(test_allocator
        PRIVATE
        ${CMAKE_SOURCE_DIR}/core/mem/allocator/src
)


add_executable(test_mpu
        test_mpu/test_mpu.c
        ${CMAKE_SOURCE_DIR}/bsp/stm32f4x/startup.c
)

target_link_libraries(test_mpu
        PRIVATE
        uart
        scheduler
        bsp
)


add_executable(test_scheduler
        test_scheduler/test_scheduler.c
        ${CMAKE_SOURCE_DIR}/bsp/stm32f4x/startup.c
)

target_link_libraries(test_scheduler
        PRIVATE
        uart
        scheduler
        bsp
)


add_executable(test_uart
        test_uart/test_uart.c
        ${CMAKE_SOURCE_DIR}/bsp/stm32f4x/startup.c
)

target_link_libraries(test_uart
        PRIVATE
        uart
        scheduler
        bsp
)

foreach(t test_allocator test_mpu test_scheduler test_uart)
    set_target_properties(${t} PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/bsp/stm32f4x/linker.ld)
endforeach()

add_flash(test_allocator)
add_flash(test_mpu)
add_flash(test_scheduler)
add_flash(test_uart)
