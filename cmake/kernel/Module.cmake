#[[
    Module.cmake - JANUS Module Helper — private

    Provides _janus_add_module(), the common implementation shared by
    janus_add_library(), janus_add_core(), and janus_add_subsys().
    Callers are responsible for any type-specific behaviour (arch subdirectory
    for subsystems, placeholder status messages for libraries, etc.).
]]

include_guard(GLOBAL)

if(NOT JANUS_PLATFORM_LOADED)
    message(FATAL_ERROR "platform/Detection.cmake must be included before kernel/Module.cmake")
endif()

if(NOT JANUS_REGISTRY_LOADED)
    message(FATAL_ERROR "Registry.cmake must be included before kernel/Module.cmake")
endif()

# ---------------------------------------------------------------------------
# _janus_add_module(NAME TYPE
#     [SOURCES      src1 src2 ...]
#     [DEPENDENCIES dep1 dep2 ...]
# )
#
# Registers NAME under TYPE in the kernel registry, then creates:
#   - an INTERFACE library (when SOURCES is empty)  or
#   - a STATIC   library (when SOURCES is provided)
#
# In both cases the standard two include directories are wired:
#   ${CMAKE_CURRENT_SOURCE_DIR}/include   (PUBLIC / INTERFACE)
#   ${CMAKE_SOURCE_DIR}/kernel/include    (PUBLIC / INTERFACE)
# ---------------------------------------------------------------------------
function(_janus_add_module NAME TYPE)
    cmake_parse_arguments(ARG "" "" "SOURCES;DEPENDENCIES" ${ARGN})

    janus_register("${NAME}" "${TYPE}" "${ARG_DEPENDENCIES}")

    if(NOT ARG_SOURCES)
        add_library("${NAME}" INTERFACE)
        target_include_directories("${NAME}" INTERFACE
            "${CMAKE_CURRENT_SOURCE_DIR}/include"
            "${CMAKE_SOURCE_DIR}/kernel/include"
        )
        if(ARG_DEPENDENCIES)
            target_link_libraries("${NAME}" INTERFACE ${ARG_DEPENDENCIES})
        endif()
        return()
    endif()

    add_library("${NAME}" STATIC ${ARG_SOURCES})
    target_include_directories("${NAME}" PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}/kernel/include"
    )
    if(ARG_DEPENDENCIES)
        target_link_libraries("${NAME}" PUBLIC ${ARG_DEPENDENCIES})
    endif()
    janus_apply_compile_flags("${NAME}")
endfunction()
