# Kernel init module - the final assembly point

file(GLOB INIT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

# kmain links to subsystems - arch includes propagate automatically via PUBLIC
janus_add_subsys(kmain SOURCES ${INIT_SOURCES}
    DEPENDENCIES
        drivers
        boot
)

# Only add paths not provided by dependencies
target_include_directories(kmain
    PUBLIC
        ${CMAKE_BINARY_DIR}/include  # Generated headers
)

