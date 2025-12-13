# Memory manager module

tinyos_add_module(memman
    DEPENDENCIES
        types
        memory
)

# Add tests if enabled
tinyos_add_tests(memman
    SOURCES
        # memman_test.c (future)
)
