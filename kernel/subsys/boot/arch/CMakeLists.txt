# Architecture dispatcher for the boot subsystem
# Dispatches to the appropriate architecture implementation

include(JanusArch)

add_subdirectory(${JANUS_TARGET_ARCH})

