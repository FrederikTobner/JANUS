file(GLOB_RECURSE MM_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

janus_add_subsys(mm
    SOURCES
        ${MM_SOURCES}
    DEPENDENCIES
        kio
        janus_contract_memmap
)

