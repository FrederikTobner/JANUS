# Memory management module

# Automatically find all source files
file(GLOB_RECURSE MM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE MM_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

janus_add_subsys(mm
    SOURCES
        ${MM_SOURCES}
    DEPENDENCIES
        memory
)

