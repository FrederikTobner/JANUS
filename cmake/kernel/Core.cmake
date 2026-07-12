# JANUS Core Layer Helper
# Provides janus_add_core() for creating kernel core services (e.g. kio)
#
# Core modules sit between the library layer (lib/) and the subsystem layer (subsys/).
# Rules:
#   - Core modules MAY depend on libs
#   - Core modules MUST NOT depend on subsystems
#   - Subsystems MAY depend on core modules
# These constraints are enforced by janus_validate_registry().

include_guard(GLOBAL)

if(NOT JANUS_PLATFORM_LOADED)
    message(FATAL_ERROR "platform/Detection.cmake must be included before kernel/Core.cmake")
endif()

if(NOT JANUS_REGISTRY_LOADED)
    message(FATAL_ERROR "Registry.cmake must be included before kernel/Core.cmake")
endif()

include(kernel/Module)

#
# Add a kernel core module
#
# Usage:
#   janus_add_core(name
#       SOURCES file1.c file2.c
#       [DEPENDENCIES dep1 dep2]   # libs only — no subsystems
#   )
#
function(janus_add_core NAME)
    cmake_parse_arguments(ARG "" "" "SOURCES;DEPENDENCIES" ${ARGN})

    _janus_add_module("${NAME}" CORE
        SOURCES      ${ARG_SOURCES}
        DEPENDENCIES ${ARG_DEPENDENCIES}
    )
endfunction()
