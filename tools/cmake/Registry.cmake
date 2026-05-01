# JANUS Tools Registry
# Tracks all tool libraries and tools and their dependencies.
# Used for dependency graph generation.

include_guard(GLOBAL)

set(TOOLS_REGISTRY_NAMES "" CACHE INTERNAL "All tools registry names" FORCE)

function(tools_register NAME TYPE DEPENDENCIES)
    set(TOOLS_REGISTRY_NAMES "${TOOLS_REGISTRY_NAMES};${NAME}" CACHE INTERNAL "All tools registry names" FORCE)
    set(TOOLS_TYPE_${NAME} ${TYPE} CACHE INTERNAL "Tools type for ${NAME}" FORCE)
    set(TOOLS_DEPS_${NAME} ${DEPENDENCIES} CACHE INTERNAL "Tools deps for ${NAME}" FORCE)
endfunction()

# Sanitize a name into a valid Mermaid node ID (replace - and . with _)
function(_tools_mermaid_id NAME OUT_VAR)
    string(REPLACE "-" "_" _id "${NAME}")
    string(REPLACE "." "_" _id "${_id}")
    set(${OUT_VAR} "${_id}" PARENT_SCOPE)
endfunction()

# Generate a Mermaid dependency graph and write it (wrapped in a fenced code block)
# to OUTPUT_FILE.  Call after all subdirectories have been added.
#
# Node types map to Mermaid shapes:
#   TOOL_LIB → rounded rectangle  name("label")
#   TOOL     → hexagon            name{{"label"}}
function(tools_write_mermaid_diagram OUTPUT_FILE)
    set(_lib_nodes "")
    set(_tool_nodes "")

    foreach(name ${TOOLS_REGISTRY_NAMES})
        if(NOT name)
            continue()
        endif()
        set(_t "${TOOLS_TYPE_${name}}")
        if(_t STREQUAL "TOOL_LIB")
            list(APPEND _lib_nodes "${name}")
        elseif(_t STREQUAL "TOOL")
            list(APPEND _tool_nodes "${name}")
        endif()
    endforeach()

    set(_registered "${TOOLS_REGISTRY_NAMES}")

    set(_d "graph TD\n")

    if(_lib_nodes)
        string(APPEND _d "  subgraph lib_layer[\"Tool Libraries\"]\n")
        foreach(_n ${_lib_nodes})
            _tools_mermaid_id("${_n}" _id)
            string(APPEND _d "    ${_id}(\"${_n}\")\n")
        endforeach()
        string(APPEND _d "  end\n")
    endif()

    if(_tool_nodes)
        string(APPEND _d "  subgraph tool_layer[\"Tools\"]\n")
        foreach(_n ${_tool_nodes})
            _tools_mermaid_id("${_n}" _id)
            string(APPEND _d "    ${_id}{{\"${_n}\"}}\n")
        endforeach()
        string(APPEND _d "  end\n")
    endif()

    foreach(name ${TOOLS_REGISTRY_NAMES})
        if(NOT name)
            continue()
        endif()
        _tools_mermaid_id("${name}" _from)
        foreach(dep ${TOOLS_DEPS_${name}})
            if(dep IN_LIST _registered)
                _tools_mermaid_id("${dep}" _to)
                string(APPEND _d "  ${_from} --> ${_to}\n")
            endif()
        endforeach()
    endforeach()

    set(_note "> *Generated at configure time — re-run `cmake -B build-tools` to regenerate.*\n\n")
    set(_content "${_note}```mermaid\n${_d}```\n")

    get_filename_component(_dir "${OUTPUT_FILE}" DIRECTORY)
    file(MAKE_DIRECTORY "${_dir}")
    file(WRITE "${OUTPUT_FILE}" "${_content}")
    message(STATUS "  Generated tools dependency diagram: ${OUTPUT_FILE}")
endfunction()

set(TOOLS_REGISTRY_LOADED TRUE CACHE INTERNAL "Tools registry loaded" FORCE)
