# Kernel buffer library
project(libkbuffer)

add_library(kbuffer STATIC)

target_include_directories(kbuffer
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
)

target_sources(kbuffer
    PRIVATE
        # Source files will be added as they are created
        # buffer.c
)

target_compile_options(kbuffer
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
