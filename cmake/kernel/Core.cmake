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
    cmake_parse_arguments(
        ARG
        ""
        ""
        "SOURCES;DEPENDENCIES"
        ${ARGN}
    )

    janus_register(${NAME} CORE "${ARG_DEPENDENCIES}")

    if(NOT ARG_SOURCES)
        add_library(${NAME} INTERFACE)
        target_include_directories(${NAME} INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/kernel/include
        )
        if(ARG_DEPENDENCIES)
            target_link_libraries(${NAME} INTERFACE ${ARG_DEPENDENCIES})
        endif()
        return()
    endif()

    add_library(${NAME} STATIC ${ARG_SOURCES})

    target_include_directories(${NAME}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/kernel/include
    )

    if(ARG_DEPENDENCIES)
        target_link_libraries(${NAME} PUBLIC ${ARG_DEPENDENCIES})
    endif()

    janus_apply_compile_flags(${NAME})
endfunction()
