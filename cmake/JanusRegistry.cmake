# JANUS Registry - Tracks all subsystems and libraries and their dependencies
# Used for post-processing validation and dependency graph generation

include_guard(GLOBAL)

# Master list of all registered names
set(JANUS_REGISTRY_NAMES "" CACHE INTERNAL "All registry names")

# Helper to register an entry
function(janus_register NAME TYPE DEPENDENCIES)
    set(JANUS_REGISTRY_NAMES "${JANUS_REGISTRY_NAMES};${NAME}" CACHE INTERNAL "All registry names" FORCE)
    set(JANUS_TYPE_${NAME} ${TYPE} CACHE INTERNAL "Janus type for ${NAME}" FORCE)
    set(JANUS_DEPS_${NAME} ${DEPENDENCIES} CACHE INTERNAL "Janus deps for ${NAME}" FORCE)
endfunction()

# Post-processing validation: ensure no subsystem depends on another subsystem
function(janus_validate_registry)
    message(STATUS "Validating dependency isolation...")
    set(subsys_names "")
    set(lib_names "")
    foreach(name ${JANUS_REGISTRY_NAMES})
        if(JANUS_TYPE_${name} STREQUAL "SUBSYS")
            list(APPEND subsys_names "${name}")
        elseif(JANUS_TYPE_${name} STREQUAL "LIB")
            list(APPEND lib_names "${name}")
        endif()
    endforeach()
    # Validate: no subsystem depends on another subsystem
    foreach(name ${subsys_names})
        if(NOT name STREQUAL "kmain")
            foreach(dep ${JANUS_DEPS_${name}})
                if(dep IN_LIST subsys_names)
                    message(FATAL_ERROR "ISOLATION VIOLATION: Subsystem '${name}' cannot depend on subsystem '${dep}'. Subsystems must be independent.")
                endif()
            endforeach()
        endif()
    endforeach()
endfunction()

set(JANUS_REGISTRY_LOADED TRUE CACHE INTERNAL "Janus registry loaded")