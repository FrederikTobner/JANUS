# Memory manager module
project(tinyos-memman)

add_library(memman STATIC)

target_include_directories(memman
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
)

target_sources(memman
    PRIVATE
        # Source files will be added in future phases
        # Physical memory manager
        # Virtual memory manager
        # Kernel heap allocator
)

target_compile_options(memman
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

target_link_libraries(memman
    PRIVATE
        types
)
