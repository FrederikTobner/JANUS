# JANUS Tools Registry
# Thin wrapper around cmake/Registry.cmake that sets up the TOOLS registry.
# The shared implementation lives one level up in cmake/Registry.cmake.

include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/Registry.cmake")

janus_registry_init(TOOLS)

# Register a tool or tool library in the TOOLS registry.
function(janus_register_tool NAME TYPE DEPENDENCIES)
    janus_registry_register(TOOLS "${NAME}" "${TYPE}" ${DEPENDENCIES})
endfunction()

# Generate the tools dependency diagram.
function(janus_write_tools_diagram OUTPUT_FILE)
    janus_registry_write_diagram(
        REGISTRY    TOOLS
        OUTPUT_FILE "${OUTPUT_FILE}"
        NOTE        "> *Generated at configure time — re-run \`cmake -B build-tools\` to regenerate.*"
        LAYERS
            "Tool Libraries|ROUNDED_RECT|TOOL_LIB"
            "Tools|HEX|TOOL"
    )
endfunction()

set(JANUS_TOOLS_REGISTRY_LOADED TRUE CACHE INTERNAL "Tools registry loaded" FORCE)
