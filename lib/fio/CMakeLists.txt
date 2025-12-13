# Kernel I/O library
project(libkio)

add_library(kio STATIC)

target_include_directories(kio
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
)

target_sources(kio
    PRIVATE
        # Source files will be added as they are created
        # io.c
)

target_compile_options(kio
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

target_link_libraries(kio
    PUBLIC
        kbuffer
)
