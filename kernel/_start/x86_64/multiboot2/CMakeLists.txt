# Entry point: x86_64 + multiboot2
# Creates the final kernel.elf executable

enable_language(ASM_NASM)

# NASM configuration
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")

# Build entry point assembly as object library
add_library(_start OBJECT
    multiboot2.asm
    boot.asm
)

message(STATUS "  Added linkable kernel target: _start (${JANUS_TARGET_ARCH}/${JANUS_BOOT_PROTOCOL})")

# Link everything into kernel.elf
janus_link_kernel(
    LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/link.ld
    LIBRARIES drivers boot
    OBJECTS _start kmain
)
