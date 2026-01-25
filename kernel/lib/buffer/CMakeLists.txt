# buffer - Character buffer operations

# Automatically find all source files
file(GLOB_RECURSE BUFFER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE BUFFER_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

janus_add_library(buffer
    SOURCES
        ${BUFFER_SOURCES}
    DEPENDENCIES
        memory
)

# Add tests if enabled
janus_add_tests(buffer
    SOURCES
        # buffer_test.c (future)
)
