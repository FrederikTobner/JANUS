# JANUS Kernel Executable Helper
# Provides janus_add_kernel() for linking kernel ELF binaries

include_guard(GLOBAL)

# Ensure platform is loaded
if(NOT JANUS_PLATFORM_LOADED)
    message(FATAL_ERROR "platform/Detection.cmake must be included before kernel/Executable.cmake")
endif()

#
# Link a kernel executable
# 
# Called from _start/${ARCH}/ to create kernel executables.
# Supports building multiple kernel binaries for different boot protocols.
#
# Usage:
#   janus_add_kernel(
#       TARGET kernel-limine.elf        # Optional, defaults to kernel.elf
#       LINKER_SCRIPT path/to/link.ld
#       DEPENDENCIES dep1 dep2
#       OBJECTS boot init               # Object libraries
#   )
#
function(janus_add_kernel)
    cmake_parse_arguments(
        ARG
        ""
        "TARGET;LINKER_SCRIPT"
        "DEPENDENCIES;OBJECTS"
        ${ARGN}
    )

    # Validate required arguments
    if(NOT ARG_LINKER_SCRIPT)
        message(FATAL_ERROR "janus_link_kernel: LINKER_SCRIPT required")
    endif()

    # Default target name
    if(NOT ARG_TARGET)
        set(ARG_TARGET "kernel.elf")
    endif()

    # Collect object files from OBJECT libraries
    set(_object_sources "")
    if(ARG_OBJECTS)
        foreach(_obj ${ARG_OBJECTS})
            list(APPEND _object_sources $<TARGET_OBJECTS:${_obj}>)
        endforeach()
    endif()

    # Create kernel executable from object libraries
    add_executable(${ARG_TARGET} ${_object_sources})

    # Link dependencies with kernel executable
    if(ARG_DEPENDENCIES)
            target_link_libraries(${ARG_TARGET} PRIVATE ${ARG_DEPENDENCIES})
    endif()

    # Include directories
    target_include_directories(${ARG_TARGET}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/kernel/include
            ${CMAKE_BINARY_DIR}/include
            ${CMAKE_SOURCE_DIR}/kernel/subsys/drivers/include
            ${CMAKE_SOURCE_DIR}/kernel/subsys/boot/include
            ${CMAKE_SOURCE_DIR}/kernel/lib/display/include
            ${CMAKE_SOURCE_DIR}/kernel/lib/fmt/include
    )

    # Linker flags — use target_link_options to APPEND to (not replace) global flags
    # This preserves CMAKE_EXE_LINKER_FLAGS set by toolchain files (e.g. --target, -fuse-ld)
    target_link_options(${ARG_TARGET} PRIVATE
        -T ${ARG_LINKER_SCRIPT}
        -nostdlib
        -static
    )
    set_target_properties(${ARG_TARGET} PROPERTIES
        LINK_DEPENDS "${ARG_LINKER_SCRIPT}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

endfunction()
