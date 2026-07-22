# aarch64 architecture implementation for interrupts subsystem

# GAS vector table in its own object library so the C compile flags are applied
# uniformly to assembly, and so it can pull the shared frame offsets header
# (<arch/impl/interrupts/frame.h>) via the C preprocessor (.S file).
add_library(interrupts_vectors OBJECT vector_table.S)
target_compile_options(interrupts_vectors PRIVATE ${JANUS_COMPILE_OPTIONS_COMMON})
target_include_directories(interrupts_vectors PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/internal"   # <arch/internal/interrupts/*.h> 
    "${CMAKE_SOURCE_DIR}/kernel/include"    # <janus/*.h> (guarded out for the assembler)
)

janus_add_arch_subsys(interrupts
    SOURCES
        init.c
        exceptions.c
        handlers.c
        $<TARGET_OBJECTS:interrupts_vectors>
    DEPENDENCIES
        janus_asm
        kio
)
