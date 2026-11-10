cmake_minimum_required(VERSION 3.31)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/arm_config.cmake)

set(FLASH openocd -f interface/stlink.cfg -f target/stm32f4x.cfg)

function(add_flash name)
    add_custom_target(flash_${name}
            COMMAND ${FLASH}
                -c "init"
                -c "reset halt"
                -c "program $<TARGET_FILE:${name}> verify reset exit"
    VERBATIM)
    add_dependencies(flash_${name} ${name})
endfunction()

project(firmware LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX ".elf")

add_compile_options(-Wall -Wextra)

add_subdirectory(regs)
add_subdirectory(bsp/stm32f4x)
add_subdirectory(core/mem/allocator)
add_subdirectory(core/scheduler)
add_subdirectory(drivers/uart)
add_subdirectory(tests)

add_executable(app app/main.c bsp/stm32f4x/startup.c)

set_target_properties(app PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/bsp/stm32f4x/linker.ld)

target_link_libraries(app PRIVATE
        bsp
        allocator
        scheduler
        uart
)

add_flash(app)
