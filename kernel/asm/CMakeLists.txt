# JANUS asm — Inline Assembly Primitives
#
# Defines the 'janus_asm' INTERFACE target.
# The include path resolves #include <asm/foo.h> through the same
# include -> arch/include -> arch/<arch>/include chain used by the
# other kernel libraries.

add_library(janus_asm INTERFACE)

target_include_directories(janus_asm
    INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/arch/${JANUS_TARGET_ARCH}/include"
)

# Register in the dependency graph so it appears in the generated diagram
janus_register(janus_asm ASM "")
