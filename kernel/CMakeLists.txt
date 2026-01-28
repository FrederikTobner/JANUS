include(JanusLibrary)
# Configure config.h from config.h.in
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/janus/config.h.in"
    "${CMAKE_BINARY_DIR}/include/janus/config.h"
    @ONLY
)

if(JANUS_BOOT_PROTOCOL STREQUAL "multiboot2")
    # Generate grub.cfg from template
    configure_file(
        ${CMAKE_SOURCE_DIR}/cmake/boot/multiboot2/grub.cfg.in
        ${CMAKE_BINARY_DIR}/grub.cfg
        @ONLY
    )
else()
    message(STATUS "Boot protocol '${JANUS_BOOT_PROTOCOL}' selected: GRUB config generation is not implemented")
endif()

# Add generated config.h to include path
include_directories("${CMAKE_BINARY_DIR}/kernel/include" "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Kernel libraries - These can only include the types from our include directory
add_subdirectory(lib)

# Architecture-specific code
add_subdirectory(arch)

# Kernel subsystems 
add_subdirectory(subsys)

# Kernel initialization (protocol-agnostic)
add_subdirectory(kmain)

# Entry point (creates kernel.elf, must be last)
add_subdirectory(_start)
