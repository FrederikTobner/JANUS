# Kernel core module

# Automatically find all source files
file(GLOB_RECURSE KERNEL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE KERNEL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

set(_tinyos_linker_script_arch_protocol "${CMAKE_CURRENT_SOURCE_DIR}/${TINYOS_TARGET_ARCH}/${TINYOS_BOOT_PROTOCOL}/linker.ld")

if(EXISTS "${_tinyos_linker_script_arch_protocol}")
    set(_tinyos_linker_script "${_tinyos_linker_script_arch_protocol}")
else()
    message(FATAL_ERROR
        "Linker script not found for TINYOS_TARGET_ARCH='${TINYOS_TARGET_ARCH}' and "
        "TINYOS_BOOT_PROTOCOL='${TINYOS_BOOT_PROTOCOL}'.\n"
        "Expected file: ${_tinyos_linker_script_arch_protocol}\n"
        "Add that file to support this combination.")
endif()

tinyos_create_kernel(
    SOURCES
        ${KERNEL_SOURCES}
    LIBRARIES
        arch           # Architecture-specific code
        boot           # Boot assembly objects
        drivers        # Hardware drivers
    LINKER_SCRIPT
        ${_tinyos_linker_script}
)

