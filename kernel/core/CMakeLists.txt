# Kernel core module

# Automatically find all source files
file(GLOB_RECURSE KERNEL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE KERNEL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

set(_janus_linker_script_arch_protocol "${CMAKE_CURRENT_SOURCE_DIR}/${JANUS_TARGET_ARCH}/${JANUS_BOOT_PROTOCOL}/linker.ld")

if(EXISTS "${_janus_linker_script_arch_protocol}")
    set(_janus_linker_script "${_janus_linker_script_arch_protocol}")
else()
    message(FATAL_ERROR
    "Linker script not found for JANUS_TARGET_ARCH='${JANUS_TARGET_ARCH}' and "
    "JANUS_BOOT_PROTOCOL='${JANUS_BOOT_PROTOCOL}'.\n"
        "Expected file: ${_janus_linker_script_arch_protocol}\n"
        "Add that file to support this combination.")
endif()

janus_create_kernel(
    SOURCES
        ${KERNEL_SOURCES}
    LIBRARIES
        arch           # Architecture-specific code
        boot           # Boot assembly objects
        drivers        # Hardware drivers
    LINKER_SCRIPT
        ${_janus_linker_script}
)

