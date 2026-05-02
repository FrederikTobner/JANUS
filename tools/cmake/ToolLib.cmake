# JANUS Tool Library Helper
# Provides janus_add_tooling_lib() for creating tool-space libraries.

include_guard(GLOBAL)

if(NOT JANUS_TOOLS_REGISTRY_LOADED)
    message(FATAL_ERROR "tools/cmake/Registry.cmake must be included before ToolLib.cmake")
endif()

#
# Add a tool library with standard configuration.
#
# Usage:
#   janus_add_tooling_lib(name
#       SOURCES file1.c file2.c
#       [DEPENDENCIES dep1 dep2]
#   )
#
function(janus_add_tooling_lib NAME)
    cmake_parse_arguments(ARG "" "" "SOURCES;DEPENDENCIES" ${ARGN})

    add_library(${NAME} STATIC ${ARG_SOURCES})

    target_include_directories(${NAME}
        PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_compile_features(${NAME} PUBLIC c_std_11)

    target_compile_options(${NAME} PRIVATE -Wall -Wextra -Wpedantic)

    if(ARG_DEPENDENCIES)
        target_link_libraries(${NAME} PUBLIC ${ARG_DEPENDENCIES})
    endif()

    janus_register_tool(${NAME} TOOL_LIB "${ARG_DEPENDENCIES}")
endfunction()
