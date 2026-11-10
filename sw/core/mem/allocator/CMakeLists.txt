add_library(allocator STATIC src/my_malloc.c)

target_include_directories(allocator PUBLIC include/)