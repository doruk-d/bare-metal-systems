add_library(uart STATIC
    src/uart.c
)

target_include_directories(uart PUBLIC
        include/
        ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(uart PRIVATE regs)