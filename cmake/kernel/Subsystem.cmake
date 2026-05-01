# JANUS Subsystem Helper
# Provides janus_add_subsys() for creating kernel subsystems (boot, drivers, mm)

include_guard(GLOBAL)

# Ensure platform is loaded
if(NOT JANUS_PLATFORM_LOADED)
    message(FATAL_ERROR "platform/Detection.cmake must be included before kernel/Subsystem.cmake")
endif()

if(NOT JANUS_REGISTRY_LOADED)
    message(FATAL_ERROR "Registry.cmake must be included before kernel/Subsystem.cmake")
endif()

#
# Add a kernel subsystem (like boot, drivers, mm)
#
# Usage:
#   janus_add_subsys(name
#       SOURCES file1.c file2.c
#       [DEPENDENCIES dep1 dep2]
#       [NO_ASM]
#   )
#
# Architecture-specific code lives in arch/ subdirectory with its own CMakeLists.txt.
# The arch CMakeLists.txt uses janus_add_arch_subsys() to add sources explicitly.
# Enforces subsystem isolation - subsystems cannot depend on other subsystems.
# Use NO_ASM for top-level aggregation modules (e.g. kmain) that must not have a
# direct edge to janus_asm in the dependency graph per the privilege model.
#
function(janus_add_subsys NAME)
    cmake_parse_arguments(
        ARG
        "NO_ASM"
        ""
        "SOURCES;DEPENDENCIES"
        ${ARGN}
    )

    # Register subsystem and dependencies in global registry
    if(ARG_NO_ASM)
        janus_register(${NAME} SUBSYS "${ARG_DEPENDENCIES}")
    else()
        janus_register(${NAME} SUBSYS "janus_asm;${ARG_DEPENDENCIES}")
    endif()

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
        # ASM layer (skip for NO_ASM targets like kmain)
        if(NOT ARG_NO_ASM)
            target_link_libraries(${NAME} INTERFACE janus_asm)
        endif()
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
    
    # ASM layer (skip for NO_ASM targets like kmain)
    if(NOT ARG_NO_ASM)
        target_link_libraries(${NAME} PUBLIC janus_asm)
    endif()

    # Add arch-specific include directories if arch exists
    if(HAS_ARCH)
        target_link_libraries(${NAME} PUBLIC ${NAME}_arch)
    endif()

    # Link dependencies (only lib allowed, not other subsystems)
    if(ARG_DEPENDENCIES)
        target_link_libraries(${NAME} PUBLIC ${ARG_DEPENDENCIES})
    endif()

    # Apply compiler flags
    janus_apply_compile_flags(${NAME})

endfunction()
