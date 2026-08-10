# JANUS Kernel Registry
# Thin wrapper around cmake/Registry.cmake that sets up the KERNEL registry.
# The shared implementation lives at the repo root in cmake/Registry.cmake.
# Mirrors tools/cmake/Registry.cmake — see that file for the TOOLS registry
# and use both as the template for a future user/cmake/Registry.cmake.

include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/Registry.cmake")

janus_registry_init(KERNEL)

# Register an entry in the kernel registry.
function(janus_register NAME TYPE DEPENDENCIES)
    janus_registry_register(KERNEL "${NAME}" "${TYPE}" ${DEPENDENCIES})
endfunction()

# Validate kernel subsystem isolation.
function(janus_validate_registry)
    janus_registry_validate(
        REGISTRY     KERNEL
        SUBSYS_TYPES SUBSYS
        CORE_TYPES   CORE
    )
endfunction()

# Generate the kernel dependency diagram.
function(janus_write_mermaid_diagram OUTPUT_FILE)
    janus_registry_write_diagram(
        REGISTRY     KERNEL
        OUTPUT_FILE  "${OUTPUT_FILE}"
        NOTE         "> *Generated for \`${JANUS_TARGET_ARCH}\` — run \`cmake --preset <${JANUS_TARGET_ARCH}-preset>\` to regenerate.*"
        DASHED_TYPES CONTRACT
        LABEL_PREFIX JANUS_CONTRACT_LABEL
        LAYERS
            "Library Layer|ROUNDED_RECT|LIB"
            "Core Layer|STADIUM|CORE"
            "Subsystem Layer|RECT|SUBSYS"
            "Protocol Libraries|SUBROUTINE|PROTOCOL_LIB"
            "Executables|HEX|EXEC"
            "ASM Layer|CYLINDER|ASM"
            "Contracts|CIRCLE|CONTRACT"
    )
endfunction()

# Guard variable checked by kernel/cmake/Library.cmake, Core.cmake, Subsystem.cmake.
set(JANUS_REGISTRY_LOADED TRUE CACHE INTERNAL "Janus registry loaded")
