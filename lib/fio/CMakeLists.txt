# fio - Formatted I/O library

# Automatically find all source files
file(GLOB_RECURSE FIO_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE FIO_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

tinyos_add_library(fio
    SOURCES
        ${FIO_SOURCES}
    DEPENDENCIES
        buffer
        types
)

# Add tests if enabled
tinyos_add_tests(fio
    SOURCES
        # fio_test.c (future)
)
