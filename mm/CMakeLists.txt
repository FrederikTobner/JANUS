# Memory management module

# Automatically find all source files
file(GLOB_RECURSE MM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE MM_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

tinyos_add_module(mm
    SOURCES
        ${MM_SOURCES}
    DEPENDENCIES
        types
        memory
)

# Add tests if enabled
tinyos_add_tests(mm
    SOURCES
        # mm_test.c (future)
)
