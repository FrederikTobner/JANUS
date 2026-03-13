# JANUS CMake Helper Functions
# Provides reusable functions for creating kernel libraries and modules

include_guard(GLOBAL)

# Ensure platform is loaded
if(NOT JANUS_PLATFORM_LOADED)
    message(FATAL_ERROR "JanusPlatform.cmake must be included before JanusSubsys.cmake")
endif()

if(NOT JANUS_REGISTRY_LOADED)
    message(FATAL_ERROR "JanusRegistry.cmake must be included before JanusSubsys.cmake")
endif()

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
    cmake_parse_arguments(
        ARG
        ""
        ""
        "SOURCES;DEPENDENCIES"
        ${ARGN}
    )

    # Register subsystem and dependencies in global registry
    janus_register(${NAME} SUBSYS "${ARG_DEPENDENCIES}")

    set(SUBSYS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    set(ALL_SOURCES ${ARG_SOURCES})

    # Export subsystem info to parent scope for janus_add_arch_subsys
    set(JANUS_CURRENT_SUBSYS_NAME ${NAME} PARENT_SCOPE)
    set(JANUS_CURRENT_SUBSYS_DIR "${SUBSYS_DIR}" PARENT_SCOPE)
    
    # Check for architecture folder with its own CMakeLists.txt
    set(ARCH_DIR "${SUBSYS_DIR}/arch")
    set(HAS_ARCH FALSE)
    
    if(EXISTS "${ARCH_DIR}/CMakeLists.txt")
        set(HAS_ARCH TRUE)
        # Include the arch CMakeLists which will call janus_add_arch_subsys
        add_subdirectory(arch)
    endif()

    # Handle empty subsystems (placeholders)
    if(NOT ALL_SOURCES)
        add_library(${NAME} INTERFACE)
        target_include_directories(${NAME} INTERFACE
            ${SUBSYS_DIR}/include
            ${CMAKE_SOURCE_DIR}/kernel/include
        )
        if(ARG_DEPENDENCIES)
            target_link_libraries(${NAME} INTERFACE ${ARG_DEPENDENCIES})
        endif()
        return()
    endif()

    # Create static library
    add_library(${NAME} STATIC ${ALL_SOURCES})

    # Include directories - PUBLIC for transitive propagation to consumers
    target_include_directories(${NAME}
        PUBLIC
            ${SUBSYS_DIR}/include
            ${CMAKE_SOURCE_DIR}/kernel/include
    )
    
    # Add arch-specific include directories if arch exists
    if(HAS_ARCH)
        target_link_libraries(${NAME} PUBLIC ${NAME}_arch)
    endif()

    # Link dependencies (only lib allowed, not other subsystems)
    if(ARG_DEPENDENCIES)
        target_link_libraries(${NAME} PUBLIC ${ARG_DEPENDENCIES})
    endif()

    # Apply compiler flags
    target_compile_options(${NAME} PRIVATE ${JANUS_COMPILE_OPTIONS_COMMON})
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${NAME} PRIVATE ${JANUS_COMPILE_OPTIONS_DEBUG})
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${NAME} PRIVATE ${JANUS_COMPILE_OPTIONS_RELEASE})
    elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
        target_compile_options(${NAME} PRIVATE ${JANUS_COMPILE_OPTIONS_MINSIZEREL})
    endif()

endfunction()
