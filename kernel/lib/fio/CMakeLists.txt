# fio - Formatted I/O library

# Automatically find all source files
file(GLOB_RECURSE FIO_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE FIO_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

janus_add_library(fio
    SOURCES
        ${FIO_SOURCES}
    DEPENDENCIES
        buffer        
)

# Add tests if enabled
janus_add_tests(fio
    SOURCES
        # fio_test.c (future)
)
