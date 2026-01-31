# JANUS CMake Helper Functions
# Provides reusable functions for creating kernel libraries and modules

include_guard(GLOBAL)

# Ensure platform is loaded
if(NOT JANUS_PLATFORM_LOADED)
    message(FATAL_ERROR "JanusPlatform.cmake must be included before JanusSubsys.cmake")
endif()

# Track all subsystems globally for isolation enforcement
define_property(GLOBAL PROPERTY JANUS_ALL_SUBSYSTEMS
    BRIEF_DOCS "List of all registered subsystems"
    FULL_DOCS "Used to enforce isolation - subsystems cannot depend on each other"
)

#
# Add a kernel subsystem (like boot, drivers, mm)
#
# Usage:
#   janus_add_subsys(name
#       SOURCES file1.c file2.c
#       [DEPENDENCIES dep1 dep2]
#   )
#
# Auto-detects arch/ folder - no need to specify HAS_ARCH!
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

    # Get list of all registered subsystems
    get_property(ALL_SUBSYSTEMS GLOBAL PROPERTY JANUS_ALL_SUBSYSTEMS)
    
    # ENFORCE: No subsystem-to-subsystem dependencies!
    # Exception: 'kmain' is the final assembly point that links all subsystems.
    if(NOT NAME STREQUAL "kmain")
        foreach(DEP ${ARG_DEPENDENCIES})
            if(DEP IN_LIST ALL_SUBSYSTEMS)
                message(FATAL_ERROR 
                    "ISOLATION VIOLATION: Subsystem '${NAME}' cannot depend on subsystem '${DEP}'.\n"
                    "Subsystems must be independent. Use 'lib' for shared code or resolve in 'kmain'.")
            endif()
        endforeach()
    endif()
    
    # Register this subsystem (BEFORE creating target, so we catch cycles)
    set_property(GLOBAL APPEND PROPERTY JANUS_ALL_SUBSYSTEMS ${NAME})

    set(SUBSYS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    set(ALL_SOURCES ${ARG_SOURCES})

    # Auto-detect architecture folder
    set(ARCH_DIR "${SUBSYS_DIR}/arch")
    set(ARCH_IMPL_DIR "${ARCH_DIR}/${JANUS_TARGET_ARCH}")
    set(HAS_ARCH FALSE)
    
    if(EXISTS "${ARCH_DIR}" AND IS_DIRECTORY "${ARCH_DIR}")
        set(HAS_ARCH TRUE)
        
        if(NOT EXISTS "${ARCH_IMPL_DIR}")
            message(FATAL_ERROR 
                "Subsystem '${NAME}' has arch/ but missing implementation for ${JANUS_TARGET_ARCH}: ${ARCH_IMPL_DIR}")
        endif()
        
        # Glob arch implementation sources (if any .c files exist)
        file(GLOB ARCH_IMPL_SOURCES "${ARCH_IMPL_DIR}/*.c")
        if(ARCH_IMPL_SOURCES)
            list(APPEND ALL_SOURCES ${ARCH_IMPL_SOURCES})
            message(STATUS "  [${NAME}] Arch sources: ${ARCH_IMPL_SOURCES}")
        endif()
    endif()

    # Handle empty subsystems (placeholders)
    if(NOT ALL_SOURCES)
        message(STATUS "  Added module: ${NAME} (placeholder, no sources yet)")
        add_library(${NAME} INTERFACE)
        target_include_directories(${NAME} INTERFACE
            ${SUBSYS_DIR}/include
            ${CMAKE_SOURCE_DIR}/kernel/include
            # Arch includes as PUBLIC - will propagate transitively
            # Safe because subsystems can't link to each other anyway!
            $<$<BOOL:${HAS_ARCH}>:${ARCH_DIR}/include>
            $<$<BOOL:${HAS_ARCH}>:${ARCH_IMPL_DIR}/include>
        )
        if(ARG_DEPENDENCIES)
            target_link_libraries(${NAME} INTERFACE ${ARG_DEPENDENCIES})
        endif()
        return()
    endif()

    # Create static library
    add_library(${NAME} STATIC ${ALL_SOURCES})

    # Include directories - all PUBLIC for transitive propagation
    # This is safe because we enforce subsystem isolation above!
    # When kmain links to drivers, it automatically gets drivers' arch includes.
    target_include_directories(${NAME}
        PUBLIC
            ${SUBSYS_DIR}/include
            ${CMAKE_SOURCE_DIR}/kernel/include
            # Arch contract headers (Tier 2) - propagates to consumers
            $<$<BOOL:${HAS_ARCH}>:${ARCH_DIR}/include>
            # Arch implementation headers (Tier 3) - propagates to consumers
            $<$<BOOL:${HAS_ARCH}>:${ARCH_IMPL_DIR}/include>
    )

    # Link dependencies (only lib allowed, not other subsystems)
    if(ARG_DEPENDENCIES)
        target_link_libraries(${NAME} PUBLIC ${ARG_DEPENDENCIES})
    endif()

    # Apply compiler flags
    target_compile_options(${NAME} PRIVATE ${JANUS_COMMON_FLAGS})
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${NAME} PRIVATE ${JANUS_DEBUG_FLAGS})
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${NAME} PRIVATE ${JANUS_RELEASE_FLAGS})
    elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
        target_compile_options(${NAME} PRIVATE ${JANUS_MINSIZEREL_FLAGS})
    endif()

    message(STATUS "  Added subsystem: ${NAME}")
endfunction()