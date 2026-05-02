# kio — kernel I/O core module (kprintf, vkprintf, kpanic)

include(kernel/Core)

file(GLOB_RECURSE KIO_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

janus_add_core(kio
    SOURCES
        ${KIO_SOURCES}
    DEPENDENCIES
        fmt
)
