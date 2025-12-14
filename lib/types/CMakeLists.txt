# types - Basic type definitions and compiler attributes
# Header-only library (INTERFACE)

add_library(types INTERFACE)

target_include_directories(types INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

message(STATUS "  Added library: types (header-only)")

# This is a header-only library
# All functionality is in include/lib/types.h and include/lib/compiler.h
