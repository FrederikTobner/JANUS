# JANUS CMake Helper Functions
# Provides reusable functions for creating kernel libraries and modules

include_guard(GLOBAL)

# Ensure platform is loaded
if(NOT JANUS_PLATFORM_LOADED)
    message(FATAL_ERROR "JanusPlatform.cmake must be included before JanusKernel.cmake")
endif()

# Print JANUS build configuration summary
#
# Link the final kernel executable
# 
# Called from boot/${ARCH}/${PROTOCOL}/ to create kernel.elf.
# The boot module owns the entry point and linker script.
#
# Usage:
#   janus_link_kernel(
#       LINKER_SCRIPT path/to/link.ld
#       LIBRARIES arch drivers          # Static libraries
#       OBJECTS boot init               # Object libraries
#   )
#
function(janus_link_kernel)
    cmake_parse_arguments(
        ARG
        ""
        "LINKER_SCRIPT"
        "LIBRARIES;OBJECTS"
        ${ARGN}
    )

    # Validate required arguments
    if(NOT ARG_LINKER_SCRIPT)
        message(FATAL_ERROR "janus_link_kernel: LINKER_SCRIPT required")
    endif()

    # Collect object files from OBJECT libraries
    set(_object_sources "")
    if(ARG_OBJECTS)
        foreach(_obj ${ARG_OBJECTS})
            list(APPEND _object_sources $<TARGET_OBJECTS:${_obj}>)
        endforeach()
    endif()

    # Create kernel executable from object libraries
    add_executable(kernel.elf ${_object_sources})

    # Link static libraries
    if(ARG_LIBRARIES)
        target_link_libraries(kernel.elf PRIVATE ${ARG_LIBRARIES})
    endif()

    # Include directories
    target_include_directories(kernel.elf
        PRIVATE
            ${CMAKE_SOURCE_DIR}/kernel/include
            ${CMAKE_BINARY_DIR}/include
            ${CMAKE_SOURCE_DIR}/kernel/subsys/drivers/include
            ${CMAKE_SOURCE_DIR}/kernel/subsys/boot/include
            ${CMAKE_SOURCE_DIR}/kernel/arch/include
    )

    # Custom linker flags with linker script
    set_target_properties(kernel.elf PROPERTIES
        LINK_FLAGS "-T ${ARG_LINKER_SCRIPT} -nostdlib -static"
        LINK_DEPENDS "${ARG_LINKER_SCRIPT}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

    message(STATUS "  Created kernel executable: kernel.elf")
    message(STATUS "    Linker script: ${ARG_LINKER_SCRIPT}")
    message(STATUS "    Libraries: ${ARG_LIBRARIES}")
    message(STATUS "    Objects: ${ARG_OBJECTS}")
endfunction()

