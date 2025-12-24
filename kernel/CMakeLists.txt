
# Configure config.h from config.h.in
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/tinyos/config.h.in"
    "${CMAKE_BINARY_DIR}/include/tinyos/config.h"
    @ONLY
)

# Add generated config.h to include path
include_directories("${CMAKE_BINARY_DIR}/kernel/include", "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Boot module
add_subdirectory(boot)

# Core kernel modules
add_subdirectory(core)

# Device drivers (future)
# add_subdirectory(drivers)

# Generate grub.cfg from template
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/grub.cfg.in
    ${CMAKE_BINARY_DIR}/grub.cfg
    @ONLY
)
