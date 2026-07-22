#[[
    Subsystem.cmake - JANUS Subsystem Helper

    Provides janus_add_subsys() for creating kernel subsystems (boot, drivers, mm)
]]

include_guard(GLOBAL)

# Ensure platform is loaded
if(NOT JANUS_PLATFORM_LOADED)
    message(FATAL_ERROR "platform/Detection.cmake must be included before kernel/Subsystem.cmake")
endif()

if(NOT JANUS_REGISTRY_LOADED)
    message(FATAL_ERROR "Registry.cmake must be included before kernel/Subsystem.cmake")
endif()

include(kernel/Module)

#
# Add a kernel subsystem (like boot, drivers, mm)
#
# Usage:
#   janus_add_subsys(name
#       SOURCES file1.c file2.c
#       [DEPENDENCIES dep1 dep2]
#   )
#
# Architecture-specific code lives in arch/ subdirectory with its own CMakeLists.txt.
# The arch CMakeLists.txt uses janus_add_arch_subsys() to add sources explicitly.
# Enforces subsystem isolation - subsystems cannot depend on other subsystems.
#
function(janus_add_subsys NAME)
    cmake_parse_arguments(ARG "" "" "SOURCES;DEPENDENCIES" ${ARGN})

    # Export subsystem identity to parent scope so janus_add_arch_subsys() can
    # read it when the arch subdirectory is processed below.
    set(JANUS_CURRENT_SUBSYS_NAME "${NAME}"                    PARENT_SCOPE)
    set(JANUS_CURRENT_SUBSYS_DIR  "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)

    set(_has_arch FALSE)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/arch/CMakeLists.txt")
        set(_has_arch TRUE)
        add_subdirectory(arch)
    endif()

    _janus_add_module("${NAME}" SUBSYS
        SOURCES      ${ARG_SOURCES}
        DEPENDENCIES ${ARG_DEPENDENCIES}
    )

    # Link the arch library only for non-placeholder (STATIC) targets.
    if(_has_arch AND ARG_SOURCES)
        target_link_libraries("${NAME}" PUBLIC "${NAME}_arch")
    endif()
endfunction()
