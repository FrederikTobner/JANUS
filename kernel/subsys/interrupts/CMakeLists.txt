# Interrupt and CPU-exception handling subsystem

file(GLOB_RECURSE INTERRUPTS_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

janus_add_subsys(interrupts
    SOURCES
        ${INTERRUPTS_SOURCES}
    DEPENDENCIES
        kio
)

# Note: arch/ is processed by janus_add_subsys via add_subdirectory(arch)
