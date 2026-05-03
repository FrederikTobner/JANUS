# kio - kernel I/O core module (kprintf, vkprintf, kpanic)

include(kernel/Core)

file(GLOB_RECURSE KIO_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

if(JANUS_TARGET_ARCH STREQUAL "x86_64")
    list(APPEND KIO_SOURCES arch/x86_64/halt.c)
elseif(JANUS_TARGET_ARCH STREQUAL "aarch64")
    list(APPEND KIO_SOURCES arch/aarch64/halt.c)
endif()

janus_add_core(kio
    SOURCES
        ${KIO_SOURCES}
    DEPENDENCIES
        fmt
        janus_asm
)
