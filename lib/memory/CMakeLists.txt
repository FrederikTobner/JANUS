# memory - Memory manipulation operations

tinyos_add_library(memory
    SOURCES
        memory.c
    DEPENDENCIES
        types
)

# Add tests if enabled
tinyos_add_tests(memory
    SOURCES
        # memory_test.c (future)
)
