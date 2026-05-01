# Only build page_tables for aarch64 for now
if(JANUS_TARGET_ARCH STREQUAL "aarch64")
    janus_add_library(page_tables
        SOURCES
            arch/aarch64/mmu.c
        DEPENDENCIES
            janus_asm
    )
else()
    add_library(page_tables INTERFACE)
endif()