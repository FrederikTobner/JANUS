# Kernel core module

# Automatically find all source files
file(GLOB_RECURSE KERNEL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE KERNEL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

tinyos_add_module(kernel
    SOURCES
        ${KERNEL_SOURCES}
    DEPENDENCIES
        types
        memory
        buffer
        fio
)
