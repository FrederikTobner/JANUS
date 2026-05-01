if(JANUS_TARGET_ARCH STREQUAL "aarch64")
    janus_add_library(page_tables
        SOURCES
            arch/aarch64/mmu.c
        DEPENDENCIES
            janus_asm
    )
elseif(JANUS_TARGET_ARCH STREQUAL "x86_64")
    janus_add_library(page_tables
        SOURCES
            arch/x86_64/mmu.c
        DEPENDENCIES
            janus_asm
    )
else()
    add_library(page_tables INTERFACE)
endif()