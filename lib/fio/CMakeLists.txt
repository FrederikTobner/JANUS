# fio - Formatted I/O library

tinyos_add_library(fio
    DEPENDENCIES
        buffer
        types
)

# Add tests if enabled
tinyos_add_tests(fio
    SOURCES
        # fio_test.c (future)
)
