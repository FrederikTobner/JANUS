list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(Registry)
include(CompilerFlags)
include(Contract)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/janus/config.h.in"
    "${CMAKE_BINARY_DIR}/include/janus/config.h"
    @ONLY
)

# Used to indicate that the kernel is being built (as opposed to userland code)
add_compile_definitions(JANUS_KERNEL=1)

add_subdirectory(asm)
add_subdirectory(lib)
add_subdirectory(core)
add_subdirectory(subsys)
add_subdirectory(kmain)
add_subdirectory(_start)
add_subdirectory(tests)

janus_validate_registry()
janus_validate_contracts()
janus_write_mermaid_diagram("${CMAKE_SOURCE_DIR}/docs/generated/deps-${JANUS_TARGET_ARCH}.md")
