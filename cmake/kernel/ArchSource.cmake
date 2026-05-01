# JANUS Architecture-Specific Source Helper
# Provides janus_add_arch_subsys() for building arch-specific driver implementations

include_guard(GLOBAL)

#
# Add architecture-specific implementation for a subsystem or kernel library
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

# Ensure platform is loaded
if(NOT JANUS_PLATFORM_LOADED)
    message(FATAL_ERROR "platform/Detection.cmake must be included before kernel/ArchSource.cmake")
endif()

function(janus_add_arch_subsys NAME)
    cmake_parse_arguments(
        ARG
        ""
        ""
        "SOURCES;DEPENDENCIES"
        ${ARGN}
    )

    set(ARCH_BASE "${CMAKE_CURRENT_SOURCE_DIR}/..")

    # Export arch sources to parent janus_add_subsys() via global property
    set_property(GLOBAL PROPERTY "JANUS_ARCH_SOURCES_${NAME}" "${ABS_SOURCES}")
    
    # Export include directories for janus_add_subsys() to apply
    set_property(GLOBAL PROPERTY "JANUS_ARCH_INCLUDES_${NAME}"
        "${CMAKE_CURRENT_SOURCE_DIR}/include"          # <arch/impl/drivers/*.h>
    )
    set_property(GLOBAL PROPERTY "JANUS_ARCH_PRIVATE_INCLUDES_${NAME}"
        "${CMAKE_CURRENT_SOURCE_DIR}/internal"         # <arch/internal/drivers/*.h>
    )

    set(ARCH_LIB_NAME "${NAME}_arch")
    add_library(${ARCH_LIB_NAME} STATIC ${ARG_SOURCES})
    target_include_directories(${ARCH_LIB_NAME} 
    PUBLIC 
        "${CMAKE_CURRENT_SOURCE_DIR}/include"          # <arch/impl/drivers/*.h>
        "${CMAKE_CURRENT_SOURCE_DIR}/../shared/include" # <arch/shared/include/*.h>
        "${CMAKE_CURRENT_SOURCE_DIR}/../include"         # <arch/include/drivers/*.h>
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/internal"         # <arch/internal/drivers/*.h>
        "${CMAKE_CURRENT_SOURCE_DIR}/../../include"         # <drivers/include/*.h>
    )
    # ASM layer — Tier 3 headers include <asm/cpu.h> etc.
    target_link_libraries(${ARCH_LIB_NAME} PUBLIC janus_asm)
    # Link dependencies (only lib allowed, not other subsystems)
    if(ARG_DEPENDENCIES)
        target_link_libraries(${ARCH_LIB_NAME} PUBLIC ${ARG_DEPENDENCIES})
    endif()

    # Apply compiler flags
    janus_apply_compile_flags(${ARCH_LIB_NAME})
endfunction()
