# JANUS Library Helper
# Provides janus_add_library() for creating kernel libraries

include_guard(GLOBAL)

if(NOT JANUS_PLATFORM_LOADED)
    message(FATAL_ERROR "platform/Detection.cmake must be included before kernel/Library.cmake")
endif()

if(NOT JANUS_REGISTRY_LOADED)
    message(FATAL_ERROR "Registry.cmake must be included before kernel/Library.cmake")
endif()

include(kernel/Module)

#
# Add a kernel library with standard configuration
#
# Usage:
#   janus_add_library(name
#       SOURCES file1.c file2.c
#       [DEPENDENCIES dep1 dep2]
#   )
#
function(janus_add_library NAME)
    cmake_parse_arguments(ARG "" "" "SOURCES;DEPENDENCIES" ${ARGN})

    if(NOT ARG_SOURCES)
        message(STATUS "  Added library: ${NAME} (placeholder, no sources yet)")
    endif()

    _janus_add_module("${NAME}" LIB
        SOURCES      ${ARG_SOURCES}
        DEPENDENCIES ${ARG_DEPENDENCIES}
    )
endfunction()
