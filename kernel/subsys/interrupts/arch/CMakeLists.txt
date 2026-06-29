# Architecture dispatcher for interrupts subsystem
# Dispatches to the appropriate architecture implementation

include(kernel/ArchSource)

add_subdirectory(${JANUS_TARGET_ARCH})
