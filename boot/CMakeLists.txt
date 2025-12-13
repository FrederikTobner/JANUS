# Boot module
project(tinyos-boot ASM_NASM)

enable_language(ASM_NASM)

add_library(boot OBJECT)

target_include_directories(boot
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(boot
    PRIVATE
        # Source files will be added as they are created
        # multiboot.asm
        # boot.asm
)

# NASM flags for Multiboot2
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")
