# JANUS asm — Inline Assembly Primitives
#
# Defines the 'janus_asm' INTERFACE target.
# The include path resolves #include <asm/foo.h> to the correct
# architecture directory for the current build target.

add_library(janus_asm INTERFACE)

target_include_directories(janus_asm
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/${JANUS_TARGET_ARCH}/include"
)

# Register in the dependency graph so it appears in the generated diagram
janus_register(janus_asm ASM "")
