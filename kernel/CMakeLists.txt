
# Configure config.h from config.h.in
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/tinyos/config.h.in"
    "${CMAKE_BINARY_DIR}/include/tinyos/config.h"
    @ONLY
)

if(TINYOS_BOOT_PROTOCOL STREQUAL "multiboot2")
    # Generate grub.cfg from template
    configure_file(
        ${CMAKE_SOURCE_DIR}/cmake/boot/multiboot2/grub.cfg.in
        ${CMAKE_BINARY_DIR}/grub.cfg
        @ONLY
    )
else()
    message(STATUS "Boot protocol '${TINYOS_BOOT_PROTOCOL}' selected: GRUB config generation is not implemented")
endif()

# Add generated config.h to include path
include_directories("${CMAKE_BINARY_DIR}/kernel/include" "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Boot module
add_subdirectory(boot)

# Core kernel module
add_subdirectory(core)

# Driver kernel module
add_subdirectory(drivers)

# Kernel libraries - These can only include the types from our include directory
add_subdirectory(lib)

add_subdirectory(arch)
