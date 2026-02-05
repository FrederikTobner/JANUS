# JANUS Architecture-Specific Subsystem Helper
# Provides janus_add_arch_subsys() for building arch-specific driver implementations

include_guard(GLOBAL)

#
# Add architecture-specific implementation for a subsystem
#
# Usage:
#   janus_add_arch_subsys(<subsystem_name>
#       SOURCES file1.c file2.c ...    # All sources (own + shared)
#   )
#
# - Each arch declares ALL its sources explicitly
# - Shared sources referenced via relative path: ../shared/...
# - Helper exports sources back to janus_add_subsys() via global properties
# - The main subsystem library includes both portable and arch-specific code
#
# Example:
#   janus_add_arch_subsys(drivers
#       SOURCES
#           serial.c
#           tty.c
#           ../shared/framebuffer.c
#   )
#
function(janus_add_arch_subsys NAME)
    cmake_parse_arguments(ARCH "" "" "SOURCES" ${ARGN})

    set(ARCH_BASE "${CMAKE_CURRENT_SOURCE_DIR}/..")

    # Convert sources to absolute paths
    set(ABS_SOURCES "")
    foreach(src ${ARCH_SOURCES})
        if(IS_ABSOLUTE "${src}")
            list(APPEND ABS_SOURCES "${src}")
        else()
            list(APPEND ABS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
        endif()
    endforeach()

    # Export arch sources to parent janus_add_subsys() via global property
    set_property(GLOBAL PROPERTY "JANUS_ARCH_SOURCES_${NAME}" "${ABS_SOURCES}")
    
    # Export include directories for janus_add_subsys() to apply
    set_property(GLOBAL PROPERTY "JANUS_ARCH_INCLUDES_${NAME}"
        "${CMAKE_CURRENT_SOURCE_DIR}/include"          # <arch/impl/drivers/*.h>
    )
    set_property(GLOBAL PROPERTY "JANUS_ARCH_PRIVATE_INCLUDES_${NAME}"
        "${CMAKE_CURRENT_SOURCE_DIR}/internal"         # <arch/internal/drivers/*.h>
    )

    message(STATUS "  [${NAME}] Arch sources (${JANUS_TARGET_ARCH}): ${ABS_SOURCES}")

endfunction()
