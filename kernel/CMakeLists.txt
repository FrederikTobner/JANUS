include(Registry)

include(kernel/Library)

# Configure config.h from config.h.in
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/janus/config.h.in"
    "${CMAKE_BINARY_DIR}/include/janus/config.h"
    @ONLY
)

# Number of pre-allocated page table pages in lib/page_tables.
# Increase if mmu_map_mmio() runs out of pool space at runtime.
set(JANUS_PAGE_TABLE_POOL_SIZE "16" CACHE STRING
    "Static pool size for lib/page_tables (number of 4 KB page table pages)")
add_compile_definitions(JANUS_PAGE_TABLE_POOL_SIZE=${JANUS_PAGE_TABLE_POOL_SIZE})


# Add generated config.h to include path
include_directories("${CMAKE_BINARY_DIR}/kernel/include" "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Architecture abstraction layer — defines janus_asm INTERFACE target
add_subdirectory(asm)

# Kernel libraries - These can only include the types from our include directory
add_subdirectory(lib)

# Kernel core services - may use libs, must NOT depend on subsystems
add_subdirectory(core)

# Kernel subsystems 
add_subdirectory(subsys)

# Kernel initialization (protocol-agnostic)
add_subdirectory(kmain)

# Entry point (creates kernel.elf, must be last)
add_subdirectory(_start)

# Validate subsystem/library registry for dependency isolation
janus_validate_registry()

# Generate dependency diagram into the docs source tree (one file per architecture)
janus_write_mermaid_diagram("${CMAKE_SOURCE_DIR}/docs/src/generated/deps-${JANUS_TARGET_ARCH}.md")
