# Kernel core module

# Automatically find all source files
file(GLOB_RECURSE KERNEL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE KERNEL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

# Create bootable kernel executable
tinyos_create_kernel(
    SOURCES
        ${KERNEL_SOURCES}
    LIBRARIES
        boot           # Boot assembly objects
        drivers        # Hardware drivers
    LINKER_SCRIPT
        ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
)

# Custom target to display kernel information
add_custom_target(kernel-info
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "Kernel Information"
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "Location: ${CMAKE_BINARY_DIR}/kernel.elf"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND readelf -h ${CMAKE_BINARY_DIR}/kernel.elf
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND readelf -l ${CMAKE_BINARY_DIR}/kernel.elf
    DEPENDS kernel.elf
    COMMENT "Displaying kernel information"
    VERBATIM
)
