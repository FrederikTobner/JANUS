# JANUS CMake Helper Functions
# Provides reusable functions for creating kernel libraries and modules

include_guard(GLOBAL)

# Ensure platform is loaded
if(NOT JANUS_PLATFORM_LOADED)
    message(FATAL_ERROR "JanusPlatform.cmake must be included before JanusKernel.cmake")
endif()

# Print JANUS build configuration summary
#
# Link a kernel executable
# 
# Called from _start/${ARCH}/ to create kernel executables.
# Supports building multiple kernel binaries for different boot protocols.
#
# Usage:
#   janus_link_kernel(
#       TARGET kernel-limine.elf        # Optional, defaults to kernel.elf
#       LINKER_SCRIPT path/to/link.ld
#       LIBRARIES arch drivers          # Static libraries
#       OBJECTS boot init               # Object libraries
#   )
#
function(janus_link_kernel)
    cmake_parse_arguments(
        ARG
        ""
        "TARGET;LINKER_SCRIPT"
        "LIBRARIES;OBJECTS"
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

    # Link static libraries
    if(ARG_LIBRARIES)
        target_link_libraries(${ARG_TARGET} PRIVATE ${ARG_LIBRARIES})
    endif()

    # Include directories
    target_include_directories(${ARG_TARGET}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/kernel/include
            ${CMAKE_BINARY_DIR}/include
            ${CMAKE_SOURCE_DIR}/kernel/subsys/drivers/include
            ${CMAKE_SOURCE_DIR}/kernel/subsys/boot/include
            ${CMAKE_SOURCE_DIR}/kernel/arch/include
    )

    # Custom linker flags with linker script
    set_target_properties(${ARG_TARGET} PROPERTIES
        LINK_FLAGS "-T ${ARG_LINKER_SCRIPT} -nostdlib -static"
        LINK_DEPENDS "${ARG_LINKER_SCRIPT}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

    message(STATUS "  Created kernel executable: ${ARG_TARGET}")
    message(STATUS "    Linker script: ${ARG_LINKER_SCRIPT}")
    message(STATUS "    Libraries: ${ARG_LIBRARIES}")
    message(STATUS "    Objects: ${ARG_OBJECTS}")
endfunction()

