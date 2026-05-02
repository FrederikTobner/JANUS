# JANUS Tool Helper
# Provides janus_add_tool() for creating tool executables.

include_guard(GLOBAL)

if(NOT JANUS_TOOLS_REGISTRY_LOADED)
    message(FATAL_ERROR "tools/cmake/Registry.cmake must be included before Tool.cmake")
endif()

#
# Add a tool executable with standard configuration.
#
# Usage:
#   janus_add_tool(name
#       SOURCES file1.c file2.c
#       [DEPENDENCIES dep1 dep2]
#   )
#
function(janus_add_tool NAME)
    cmake_parse_arguments(ARG "" "" "SOURCES;DEPENDENCIES" ${ARGN})

    add_executable(${NAME} ${ARG_SOURCES})

    target_compile_features(${NAME} PRIVATE c_std_11)

    target_compile_options(${NAME} PRIVATE -Wall -Wextra)

    if(ARG_DEPENDENCIES)
        target_link_libraries(${NAME} PRIVATE ${ARG_DEPENDENCIES})
    endif()

    janus_register_tool(${NAME} TOOL "${ARG_DEPENDENCIES}")

    install(TARGETS ${NAME} RUNTIME DESTINATION bin)
endfunction()
