# Device drivers module
project(janus-drivers)

# Automatically find all source files
file(GLOB_RECURSE DRIVERS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

janus_add_module(drivers
    SOURCES
        ${DRIVERS_SOURCES}
)

target_include_directories(drivers
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/kernel/include
)

target_link_libraries(drivers
    PUBLIC
        arch
)
