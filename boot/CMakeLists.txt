# Boot module

enable_language(ASM_NASM)

# Boot module (will contain assembly files)
add_library(boot INTERFACE)

target_include_directories(boot
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

message(STATUS "  Added module: boot (placeholder, no sources yet)")

# NASM configuration for future assembly files
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")
