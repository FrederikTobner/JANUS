# memory - Memory manipulation operations

add_library(memory STATIC
    memory.c
)

target_include_directories(memory PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(memory PUBLIC
    types
)

# Compiler flags for memory operations
target_compile_options(memory PRIVATE
    -target x86_64-elf
    -nostdlib
    -ffreestanding
    -g3 -gdwarf-4
    -O0
    -Wall -Wextra
    -fno-stack-protector
    -mno-red-zone
)
