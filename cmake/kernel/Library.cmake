# JANUS Library Helper
# Provides janus_add_library() for creating kernel libraries

include_guard(GLOBAL)

# Ensure platform is loaded
if(NOT JANUS_PLATFORM_LOADED)
    message(FATAL_ERROR "platform/Detection.cmake must be included before kernel/Library.cmake")
endif()

if(NOT JANUS_REGISTRY_LOADED)
    message(FATAL_ERROR "Registry.cmake must be included before kernel/Library.cmake")
endif()

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
    cmake_parse_arguments(
        ARG                    # Prefix for parsed arguments
        ""                     # Options (boolean flags)
        ""                     # Single-value arguments
        "SOURCES;DEPENDENCIES" # Multi-value arguments
        ${ARGN}
    )

    # Validate required arguments
    if(NOT ARG_SOURCES)
        # Allow empty SOURCES for placeholder modules
        message(STATUS "  Added library: ${NAME} (placeholder, no sources yet)")
        add_library(${NAME} INTERFACE)
        
        # Include directories for INTERFACE library
        target_include_directories(${NAME}
            INTERFACE
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_SOURCE_DIR}/include
        )
        
        # Link dependencies
        if(ARG_DEPENDENCIES)
            target_link_libraries(${NAME} INTERFACE ${ARG_DEPENDENCIES})
        endif()
        
        janus_register(${NAME} LIB "${ARG_DEPENDENCIES}")
        return()
    endif()

    # Create static library
    add_library(${NAME} STATIC ${ARG_SOURCES})

    janus_register(${NAME} LIB "${ARG_DEPENDENCIES}")

    # Standard include directories
    target_include_directories(${NAME}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/include
    )

    # Link dependencies
    if(ARG_DEPENDENCIES)
        target_link_libraries(${NAME} PUBLIC ${ARG_DEPENDENCIES})
    endif()

    # Apply compiler flags
    janus_apply_compile_flags(${NAME})
endfunction()
