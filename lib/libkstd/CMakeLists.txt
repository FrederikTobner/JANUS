# Kernel standard library subset
project(libkstd)

add_library(kstd STATIC)

target_include_directories(kstd
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
)

target_sources(kstd
    PRIVATE
        # Source files will be added as they are created
        # stdint.c
        # stddef.c
        # string.c
)

target_compile_options(kstd
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
