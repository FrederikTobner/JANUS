# Memory manager module
project(tinyos-memoryman)

add_library(memoryman STATIC)

target_include_directories(memoryman
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
)

target_sources(memoryman
    PRIVATE
        # Source files will be added in future phases
        # Physical memory manager
        # Virtual memory manager
        # Kernel heap allocator
)

target_compile_options(memoryman
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

target_link_libraries(memoryman
    PRIVATE
        kstd
)
