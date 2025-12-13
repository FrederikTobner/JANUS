# x86_64 architecture support
project(tinyos-arch-x86_64)

add_library(arch_x86_64 STATIC)

target_include_directories(arch_x86_64
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/arch/include
        ${CMAKE_SOURCE_DIR}/include
)

target_sources(arch_x86_64
    PRIVATE
        # Source files will be added as they are created
        # io.c
        # serial.c
        # vga.c
        # cpu.c
)

target_compile_options(arch_x86_64
    PRIVATE
        -target x86_64-elf
        -nostdlib
        -ffreestanding
        -g3 -gdwarf-4
        -O0
        -Wall -Wextra
        -fno-stack-protector
        -mno-red-zone
        -mcmodel=kernel
)
