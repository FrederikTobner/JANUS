# JANUS CMake Helper Functions
# Provides reusable functions for creating kernel libraries and modules

include_guard(GLOBAL)

# Ensure platform is loaded
if(NOT JANUS_PLATFORM_LOADED)
    message(FATAL_ERROR "JanusPlatform.cmake must be included before JanusSubsys.cmake")
endif()

#
#
# Add a kernel subsystem (like boot, drivers, mm)
# 
# Usage:
#   janus_add_subsys(name
#       SOURCES file1.c file2.c
#       [DEPENDENCIES dep1 dep2]
#   )
#
function(janus_add_subsys NAME)
    cmake_parse_arguments(
        ARG
        ""
        ""
        "SOURCES;DEPENDENCIES"
        ${ARGN}
    )

    # Validate required arguments
    if(NOT ARG_SOURCES)
        # Allow empty SOURCES for placeholder modules
        message(STATUS "  Added module: ${NAME} (placeholder, no sources yet)")
        add_library(${NAME} INTERFACE)
        
        # Include directories for INTERFACE library
        target_include_directories(${NAME}
            INTERFACE
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_SOURCE_DIR}/kernel/include
        )
        
        # Link dependencies
        if(ARG_DEPENDENCIES)
            target_link_libraries(${NAME} INTERFACE ${ARG_DEPENDENCIES})
        endif()
        
        return()
    endif()

    # Create static library (modules are libraries)
    add_library(${NAME} STATIC ${ARG_SOURCES})

    # Standard include directories
    target_include_directories(${NAME}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}kernel/include
    )

    # Link dependencies
    if(ARG_DEPENDENCIES)
        target_link_libraries(${NAME} PUBLIC ${ARG_DEPENDENCIES})
    endif()

    # Apply common compiler flags
    target_compile_options(${NAME} PRIVATE ${JANUS_COMMON_FLAGS})
    
    # Apply build-type specific flags
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${NAME} PRIVATE ${JANUS_DEBUG_FLAGS})
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${NAME} PRIVATE ${JANUS_RELEASE_FLAGS})
    elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
        target_compile_options(${NAME} PRIVATE ${JANUS_MINSIZEREL_FLAGS})
    endif()

    message(STATUS "  Added module: ${NAME}")
endfunction()