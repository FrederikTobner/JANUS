# buffer - Character buffer operations

tinyos_add_library(buffer
    DEPENDENCIES
        memory
        types
)

# Add tests if enabled
tinyos_add_tests(buffer
    SOURCES
        # buffer_test.c (future)
)
