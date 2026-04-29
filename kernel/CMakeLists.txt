include(Registry)

include(kernel/Library)

# Configure config.h from config.h.in
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/janus/config.h.in"
    "${CMAKE_BINARY_DIR}/include/janus/config.h"
    @ONLY
)


# Add generated config.h to include path
include_directories("${CMAKE_BINARY_DIR}/kernel/include" "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Kernel libraries - These can only include the types from our include directory
add_subdirectory(lib)

# Kernel subsystems 
add_subdirectory(subsys)

# Kernel initialization (protocol-agnostic)
add_subdirectory(kmain)

# Entry point (creates kernel.elf, must be last)
add_subdirectory(_start)

# Validate subsystem/library registry for dependency isolation
janus_validate_registry()

# Generate dependency diagram into the docs source tree
janus_write_mermaid_diagram("${CMAKE_SOURCE_DIR}/docs/src/generated/deps.md")
