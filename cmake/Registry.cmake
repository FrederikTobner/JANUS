# JANUS Registry - Tracks all subsystems and libraries and their dependencies
# Used for post-processing validation and dependency graph generation

include_guard(GLOBAL)

# Master list of all registered names
set(JANUS_REGISTRY_NAMES "" CACHE INTERNAL "All registry names" FORCE)

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


# Sanitize a name into a valid Mermaid node ID (replace - and . with _)
function(_janus_mermaid_id NAME OUT_VAR)
    string(REPLACE "-" "_" _id "${NAME}")
    string(REPLACE "." "_" _id "${_id}")
    set(${OUT_VAR} "${_id}" PARENT_SCOPE)
endfunction()

# Generate a Mermaid dependency graph and write it (wrapped in a fenced code block)
# to OUTPUT_FILE.  Call this after janus_validate_registry() so all targets are present.
#
# Node types map to Mermaid shapes:
#   LIB          → rounded rectangle  name("label")
#   SUBSYS       → rectangle          name["label"]
#   PROTOCOL_LIB → subroutine box     name[["label"]]
#   EXEC         → hexagon            name{{"label"}}
#
# Edges are only emitted when the dependency target is itself registered, so
# unresolved / future deps (e.g. mm → memory) silently produce no dangling edge.
function(janus_write_mermaid_diagram OUTPUT_FILE)
    # Bucket nodes by type
    set(_lib_nodes "")
    set(_subsys_nodes "")
    set(_proto_nodes "")
    set(_exec_nodes "")

    foreach(name ${JANUS_REGISTRY_NAMES})
        if(NOT name)
            continue()
        endif()
        set(_t "${JANUS_TYPE_${name}}")
        if(_t STREQUAL "LIB")
            list(APPEND _lib_nodes "${name}")
        elseif(_t STREQUAL "SUBSYS")
            list(APPEND _subsys_nodes "${name}")
        elseif(_t STREQUAL "PROTOCOL_LIB")
            list(APPEND _proto_nodes "${name}")
        elseif(_t STREQUAL "EXEC")
            list(APPEND _exec_nodes "${name}")
        endif()
    endforeach()

    # Set of all registered names for edge filtering
    set(_registered "${JANUS_REGISTRY_NAMES}")

    # ── Build graph string ──────────────────────────────────────────────────
    set(_d "graph TD\n")

    if(_lib_nodes)
        string(APPEND _d "  subgraph lib_layer[\"Library Layer\"]\n")
        foreach(_n ${_lib_nodes})
            _janus_mermaid_id("${_n}" _id)
            string(APPEND _d "    ${_id}(\"${_n}\")\n")
        endforeach()
        string(APPEND _d "  end\n")
    endif()

    if(_subsys_nodes)
        string(APPEND _d "  subgraph subsys_layer[\"Subsystem Layer\"]\n")
        foreach(_n ${_subsys_nodes})
            _janus_mermaid_id("${_n}" _id)
            string(APPEND _d "    ${_id}[\"${_n}\"]\n")
        endforeach()
        string(APPEND _d "  end\n")
    endif()

    if(_proto_nodes)
        string(APPEND _d "  subgraph proto_layer[\"Protocol Libraries\"]\n")
        foreach(_n ${_proto_nodes})
            _janus_mermaid_id("${_n}" _id)
            string(APPEND _d "    ${_id}[[\"${_n}\"]]\n")
        endforeach()
        string(APPEND _d "  end\n")
    endif()

    if(_exec_nodes)
        string(APPEND _d "  subgraph exec_layer[\"Executables\"]\n")
        foreach(_n ${_exec_nodes})
            _janus_mermaid_id("${_n}" _id)
            string(APPEND _d "    ${_id}{{\"${_n}\"}}\n")
        endforeach()
        string(APPEND _d "  end\n")
    endif()

    # Emit edges (skip deps that are not registered)
    foreach(name ${JANUS_REGISTRY_NAMES})
        if(NOT name)
            continue()
        endif()
        _janus_mermaid_id("${name}" _from)
        foreach(dep ${JANUS_DEPS_${name}})
            if(dep IN_LIST _registered)
                _janus_mermaid_id("${dep}" _to)
                string(APPEND _d "  ${_from} --> ${_to}\n")
            endif()
        endforeach()
    endforeach()

    # ── Write output ────────────────────────────────────────────────────────
    set(_note "> *Generated for `${JANUS_TARGET_ARCH}` — run `cmake --preset <preset>` to regenerate for a different architecture.*\n\n")
    set(_content "${_note}```mermaid\n${_d}```\n")

    get_filename_component(_dir "${OUTPUT_FILE}" DIRECTORY)
    file(MAKE_DIRECTORY "${_dir}")
    file(WRITE "${OUTPUT_FILE}" "${_content}")
    message(STATUS "  Generated: ${OUTPUT_FILE}")
endfunction()

set(JANUS_REGISTRY_LOADED TRUE CACHE INTERNAL "Janus registry loaded")
