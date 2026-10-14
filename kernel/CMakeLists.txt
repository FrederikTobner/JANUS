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

option(JANUS_ENABLE_KTEST "Compile and run the in-kernel ktest suite" OFF)
if(JANUS_ENABLE_KTEST)
    add_compile_definitions(JANUS_ENABLE_KTEST=1)
endif()

# Explicit in-kernel ktest case list. Modules append to this list when the
# optional runtime is enabled so the final linker sees a deterministic registry.
set(JANUS_KTEST_CASE_SOURCES "" CACHE INTERNAL "Explicit in-kernel ktest case sources")

add_subdirectory(asm)
add_subdirectory(lib)
add_subdirectory(core)
add_subdirectory(subsys)
add_subdirectory(tests)
add_subdirectory(kmain)
add_subdirectory(_start)

janus_validate_registry()
janus_validate_contracts()
janus_write_mermaid_diagram("${CMAKE_SOURCE_DIR}/docs/generated/deps-${JANUS_TARGET_ARCH}.md")
