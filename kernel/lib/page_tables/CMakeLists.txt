# Number of pre-allocated page table pages in the library. 
# Increase if mmu_map_mmio() runs out of pool space at runtime.
set(JANUS_PAGE_TABLE_POOL_SIZE "16" CACHE STRING
    "Static pool size for lib/page_tables (number of 4 KB page table pages)")
add_compile_definitions(JANUS_PAGE_TABLE_POOL_SIZE=${JANUS_PAGE_TABLE_POOL_SIZE})

file(GLOB_RECURSE PAGE_TABLE_ARCH_SPECIFIC_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/arch/${JANUS_TARGET_ARCH}/*.c")

janus_add_library(page_tables
     SOURCES
         ${PAGE_TABLE_ARCH_SPECIFIC_SOURCES}
     DEPENDENCIES
         janus_asm
)