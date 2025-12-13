# Kernel module
project(tinyos-kernel)

add_library(kernel_obj OBJECT)

target_include_directories(kernel_obj
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
)

target_sources(kernel_obj
    PRIVATE
        # Source files will be added as they are created
        # main.c
        # panic.c
        # init.c
)

target_compile_options(kernel_obj
    PRIVATE
        -target x86_64-elf
        -nostdlib
        -ffreestanding
        -g3 -gdwarf-4
        -O0
        -Wall -Wextra
        -fno-stack-protector
        -mno-red-zone
)
