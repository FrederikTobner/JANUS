# memory - Memory manipulation operations

# Automatically find all source files
file(GLOB_RECURSE MEMORY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE MEMORY_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

janus_add_library(memory
    SOURCES
        ${MEMORY_SOURCES}
)

# Add tests if enabled
janus_add_tests(memory
    SOURCES
        # memory_test.c (future)
)
