# JANUS Registry - Tracks all subsystems and libraries and their dependencies
# Used for post-processing validation and dependency graph generation

include_guard(GLOBAL)

# Master list of all registered names
set(JANUS_REGISTRY_NAMES "" CACHE INTERNAL "All registry names")

# Helper to register an entry (robust, no duplicates)
function(janus_register NAME TYPE DEPENDENCIES)
    # TYPE: SUBSYS or LIB
    # Only register once
    list(FIND JANUS_REGISTRY_NAMES "${NAME}" already_registered_idx)
    if(already_registered_idx EQUAL -1)
        set(JANUS_REGISTRY_NAMES "${JANUS_REGISTRY_NAMES};${NAME}" CACHE INTERNAL "All registry names" FORCE)
        set(JANUS_TYPE_${NAME} ${TYPE} CACHE INTERNAL "Janus type for ${NAME}" FORCE)
        set(JANUS_DEPS_${NAME} ${DEPENDENCIES} CACHE INTERNAL "Janus deps for ${NAME}" FORCE)
    endif()
endfunction()

# Post-processing validation: ensure no subsystem depends on another subsystem
function(janus_validate_registry)
    message(STATUS "Validating registry (list/map based)")
    set(subsys_names "")
    set(lib_names "")
    foreach(name ${JANUS_REGISTRY_NAMES})
        if(JANUS_TYPE_${name} STREQUAL "SUBSYS")
            list(APPEND subsys_names "${name}")
        elseif(JANUS_TYPE_${name} STREQUAL "LIB")
            list(APPEND lib_names "${name}")
        endif()
    endforeach()
    message(STATUS "All subsys: ${subsys_names}")
    message(STATUS "All libs: ${lib_names}")
    # Print all entries and their dependencies
    foreach(name ${JANUS_REGISTRY_NAMES})
        message(STATUS "Registry: ${name} [${JANUS_TYPE_${name}}] deps: ${JANUS_DEPS_${name}}")
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