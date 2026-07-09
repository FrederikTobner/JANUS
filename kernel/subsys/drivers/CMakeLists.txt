# Portable driver logic

file(GLOB_RECURSE DRIVER_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

janus_add_subsys(drivers 
    SOURCES 
        ${DRIVER_SOURCES}
    DEPENDENCIES
        janus_contract_display
)
