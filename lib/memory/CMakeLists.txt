# memory - Memory manipulation operations

# Automatically find all source files
file(GLOB_RECURSE MEMORY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE MEMORY_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

tinyos_add_library(memory
    SOURCES
        ${MEMORY_SOURCES}
    DEPENDENCIES
        types
)

# Add tests if enabled
tinyos_add_tests(memory
    SOURCES
        # memory_test.c (future)
)
