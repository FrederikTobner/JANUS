# Memory manager module

tinyos_add_module(memman
    DEPENDENCIES
        types
        memory
)

# Add tests if enabled
tinyos_add_tests(mm
    SOURCES
        # mm_test.c (future)
)
