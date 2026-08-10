#[[ 
    Registry.cmake — JANUS Generic Registry
    Parameterised registry functions shared across every build space (kernel,
    tools, and future user/stdlib spaces). Each registry is identified by a
    short REGISTRY name (e.g. KERNEL, TOOLS).
    All CMake cache variables are namespaced as JANUS_REG_<REGISTRY>_*.

    This file only contains the generic engine. Each space wraps it with its
    own thin file that calls janus_registry_init(<NAME>) and defines
    conveniently-named functions — see kernel/cmake/Registry.cmake and
    tools/cmake/Registry.cmake for the two existing wrappers, which are
    identical in shape and are the template for a future user/cmake/Registry.cmake.
]]

include_guard(GLOBAL)

# Reset a named registry.  Call once per configure before any
# janus_registry_register() calls for that registry.
function(janus_registry_init REGISTRY)
    set("JANUS_REG_${REGISTRY}_NAMES" "" CACHE INTERNAL "" FORCE)
endfunction()

# Register a named entry.
#   janus_registry_register(<registry> <name> <type> [<dep> ...])
function(janus_registry_register REGISTRY NAME TYPE)
    set(_pfx "JANUS_REG_${REGISTRY}")
    set(_names "${${_pfx}_NAMES}")
    list(APPEND _names "${NAME}")
    set("${_pfx}_NAMES"          "${_names}" CACHE INTERNAL "" FORCE)
    set("${_pfx}_TYPE_${NAME}"   "${TYPE}"   CACHE INTERNAL "" FORCE)
    set("${_pfx}_DEPS_${NAME}"   ${ARGN}     CACHE INTERNAL "" FORCE)
endfunction()

# Validate dependency isolation.
#   janus_registry_validate(REGISTRY <r> SUBSYS_TYPES <t>... [CORE_TYPES <t>...])
#
# Rules enforced:
#   * No entry whose type is in SUBSYS_TYPES (except "kmain") may depend on
#     another SUBSYS_TYPES entry.
#   * No entry whose type is in CORE_TYPES may depend on a SUBSYS_TYPES entry.
function(janus_registry_validate)
    cmake_parse_arguments(ARG "" "REGISTRY" "SUBSYS_TYPES;CORE_TYPES" ${ARGN})
    message(STATUS "Validating dependency isolation...")
    set(_pfx "JANUS_REG_${ARG_REGISTRY}")
    set(_names "${${_pfx}_NAMES}")

    set(_subsys_names "")
    foreach(name ${_names})
        if("${${_pfx}_TYPE_${name}}" IN_LIST ARG_SUBSYS_TYPES)
            list(APPEND _subsys_names "${name}")
        endif()
    endforeach()

    foreach(name ${_subsys_names})
        if(NOT name STREQUAL "kmain")
            foreach(dep ${${_pfx}_DEPS_${name}})
                if(dep IN_LIST _subsys_names)
                    message(FATAL_ERROR
                        "ISOLATION VIOLATION: Subsystem '${name}' cannot depend on "
                        "subsystem '${dep}'. Subsystems must be independent.")
                endif()
            endforeach()
        endif()
    endforeach()

    foreach(name ${_names})
        if("${${_pfx}_TYPE_${name}}" IN_LIST ARG_CORE_TYPES)
            foreach(dep ${${_pfx}_DEPS_${name}})
                if(dep IN_LIST _subsys_names)
                    message(FATAL_ERROR
                        "ISOLATION VIOLATION: Core module '${name}' cannot depend on "
                        "subsystem '${dep}'. Core modules may only depend on libs.")
                endif()
            endforeach()
        endif()
    endforeach()
endfunction()

# Internal: sanitize a name into a valid Mermaid node ID.
function(_janus_registry_mermaid_id NAME OUT_VAR)
    string(REPLACE "-" "_" _id "${NAME}")
    string(REPLACE "." "_" _id "${_id}")
    set(${OUT_VAR} "${_id}" PARENT_SCOPE)
endfunction()

# Internal: map a shape alias to Mermaid open/close delimiters.
# Supported aliases:
#   ROUNDED_RECT  name("label")     — rounded rectangle
#   STADIUM       name(["label"])   — stadium / pill
#   RECT          name["label"]     — rectangle
#   SUBROUTINE    name[["label"]]   — subroutine / double rectangle
#   HEX           name{{"label"}}   — hexagon
#   CYLINDER      name[("label")]   — cylinder / database
#   CIRCLE        name(("label"))   — circle
function(_janus_registry_mermaid_shape SHAPE OUT_OPEN OUT_CLOSE)
    if("${SHAPE}" STREQUAL "ROUNDED_RECT")
        set(${OUT_OPEN}  "("  PARENT_SCOPE)
        set(${OUT_CLOSE} ")"  PARENT_SCOPE)
    elseif("${SHAPE}" STREQUAL "STADIUM")
        set(${OUT_OPEN}  "([" PARENT_SCOPE)
        set(${OUT_CLOSE} "])" PARENT_SCOPE)
    elseif("${SHAPE}" STREQUAL "RECT")
        set(${OUT_OPEN}  "["  PARENT_SCOPE)
        set(${OUT_CLOSE} "]"  PARENT_SCOPE)
    elseif("${SHAPE}" STREQUAL "SUBROUTINE")
        set(${OUT_OPEN}  "[[" PARENT_SCOPE)
        set(${OUT_CLOSE} "]]" PARENT_SCOPE)
    elseif("${SHAPE}" STREQUAL "HEX")
        set(${OUT_OPEN}  "{{" PARENT_SCOPE)
        set(${OUT_CLOSE} "}}" PARENT_SCOPE)
    elseif("${SHAPE}" STREQUAL "CYLINDER")
        set(${OUT_OPEN}  "[(" PARENT_SCOPE)
        set(${OUT_CLOSE} ")]" PARENT_SCOPE)
    elseif("${SHAPE}" STREQUAL "CIRCLE")
        set(${OUT_OPEN}  "((" PARENT_SCOPE)
        set(${OUT_CLOSE} "))" PARENT_SCOPE)
    else()
        message(FATAL_ERROR
            "Unknown Mermaid shape alias '${SHAPE}'. "
            "Valid: ROUNDED_RECT STADIUM RECT SUBROUTINE HEX CYLINDER CIRCLE")
    endif()
endfunction()

# Generate a Mermaid dependency graph and write it to OUTPUT_FILE.
#
# LAYERS is a list of layer specification strings, each with the format:
#   "Layer Label|SHAPE_ALIAS|TYPE1,TYPE2,..."
# SHAPE_ALIAS must be one of the aliases accepted by
# _janus_registry_mermaid_shape.
#
# DASHED_TYPES  — types whose incoming edges are rendered as dashed arrows.
# LABEL_PREFIX  — CMake variable prefix used for per-node label overrides.
#                 For each node <n>, if ${LABEL_PREFIX}_<n> is non-empty,
#                 that value is used as the display label instead of <n>.
# NOTE          — optional header line written above the fenced code block.
function(janus_registry_write_diagram)
    cmake_parse_arguments(
        ARG
        ""
        "REGISTRY;OUTPUT_FILE;NOTE;LABEL_PREFIX"
        "DASHED_TYPES;LAYERS"
        ${ARGN}
    )

    set(_pfx "JANUS_REG_${ARG_REGISTRY}")
    set(_names "${${_pfx}_NAMES}")

    set(_d "graph TD\n")

    # One subgraph per layer spec.
    foreach(_spec ${ARG_LAYERS})
        # Parse "Label|SHAPE|TYPE1,TYPE2"
        string(REPLACE "|" ";" _parts "${_spec}")
        list(GET _parts 0 _layer_label)
        list(GET _parts 1 _shape_name)
        list(GET _parts 2 _types_csv)
        string(REPLACE "," ";" _layer_types "${_types_csv}")

        _janus_registry_mermaid_shape("${_shape_name}" _open _close)

        set(_layer_nodes "")
        foreach(name ${_names})
            if("${${_pfx}_TYPE_${name}}" IN_LIST _layer_types)
                list(APPEND _layer_nodes "${name}")
            endif()
        endforeach()

        if(NOT _layer_nodes)
            continue()
        endif()

        string(REPLACE " " "_" _subgraph_id "${_layer_label}")
        string(TOLOWER "${_subgraph_id}" _subgraph_id)

        string(APPEND _d "  subgraph ${_subgraph_id}[\"${_layer_label}\"]\n")
        foreach(_n ${_layer_nodes})
            _janus_registry_mermaid_id("${_n}" _id)
            set(_label "${_n}")
            if(ARG_LABEL_PREFIX)
                set(_label_var "${ARG_LABEL_PREFIX}_${_n}")
                if(${_label_var})
                    set(_label "${${_label_var}}")
                endif()
            endif()
            string(APPEND _d "    ${_id}${_open}\"${_label}\"${_close}\n")
        endforeach()
        string(APPEND _d "  end\n")
    endforeach()

    # Edges — skip deps not present in this registry.
    foreach(name ${_names})
        _janus_registry_mermaid_id("${name}" _from)
        foreach(dep ${${_pfx}_DEPS_${name}})
            if(dep IN_LIST _names)
                _janus_registry_mermaid_id("${dep}" _to)
                if("${${_pfx}_TYPE_${dep}}" IN_LIST ARG_DASHED_TYPES)
                    string(APPEND _d "  ${_from} -.-> ${_to}\n")
                else()
                    string(APPEND _d "  ${_from} --> ${_to}\n")
                endif()
            endif()
        endforeach()
    endforeach()

    if(ARG_NOTE)
        set(_header "${ARG_NOTE}\n\n")
    else()
        set(_header "")
    endif()

    set(_content "${_header}\`\`\`mermaid\n${_d}\`\`\`\n")

    get_filename_component(_dir "${ARG_OUTPUT_FILE}" DIRECTORY)
    file(MAKE_DIRECTORY "${_dir}")
    file(WRITE "${ARG_OUTPUT_FILE}" "${_content}")
    message(STATUS "Generated dependency graph in: ${ARG_OUTPUT_FILE}")
endfunction()
