file(GLOB_RECURSE KMALLOC_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

janus_add_core(kmalloc
    SOURCES
        ${KMALLOC_SOURCES}
    DEPENDENCIES
        mem
        kio
)
