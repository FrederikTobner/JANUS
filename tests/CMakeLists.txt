# TinyOS tests
project(tinyos-tests)

enable_testing()

# Unit tests
add_subdirectory(unit)

# Integration tests
add_subdirectory(integration)
