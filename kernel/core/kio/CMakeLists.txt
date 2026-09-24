file(GLOB_RECURSE KIO_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

janus_add_core(kio
    SOURCES
        ${KIO_SOURCES}
    DEPENDENCIES
        fmt
        janus_asm
)


