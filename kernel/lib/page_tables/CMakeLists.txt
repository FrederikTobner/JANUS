# Number of pre-allocated page table pages in the library. 
# Increase if mmu_map_mmio() runs out of pool space at runtime.
set(JANUS_PAGE_TABLE_POOL_SIZE "16" CACHE STRING
    "Static pool size for lib/page_tables (number of 4 KB page table pages)")
add_compile_definitions(JANUS_PAGE_TABLE_POOL_SIZE=${JANUS_PAGE_TABLE_POOL_SIZE})

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
