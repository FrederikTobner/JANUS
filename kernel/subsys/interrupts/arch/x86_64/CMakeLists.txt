# x86_64 architecture implementation for interrupts subsystem

# subsys/ is configured before _start/ (which also enables NASM), so enable the
# assembler here. enable_language is idempotent; the object-format settings
# mirror kernel/_start/x86_64/CMakeLists.txt.
enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_COMPILE_OBJECT
    "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")

# Entry stubs in their own object library so the C compile flags (which NASM
# cannot parse) are not applied to assembly sources.
add_library(interrupts_isr OBJECT isr.asm)

janus_add_arch_subsys(interrupts
    SOURCES
        init.c
        gdt.c
        idt.c
        handlers.c
        $<TARGET_OBJECTS:interrupts_isr>
    DEPENDENCIES
        janus_asm
        kio
)
