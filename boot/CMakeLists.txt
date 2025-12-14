# Boot module

enable_language(ASM_NASM)

# NASM configuration
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")

# Build boot assembly files as object library
add_library(boot OBJECT
    multiboot.asm
    boot.asm
)

target_include_directories(boot
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

message(STATUS "  Added module: boot")
