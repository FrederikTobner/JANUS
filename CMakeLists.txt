cmake_minimum_required(VERSION 3.20)
project(TinyOS VERSION 0.1.0 LANGUAGES C ASM_NASM)

# Set C standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)  # Allow GNU extensions

# Include TinyOS platform detection and helper functions
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(TinyOSPlatform)
include(TinyOSHelpers)

# Generate build timestamp
string(TIMESTAMP TINYOS_BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP TINYOS_BUILD_TIME "%H:%M:%S")

# Configure config.h from config.h.in
configure_file(
    "${CMAKE_SOURCE_DIR}/include/tinyos/config.h.in"
    "${CMAKE_BINARY_DIR}/include/tinyos/config.h"
    @ONLY
)

# Add generated config.h to include path
include_directories("${CMAKE_BINARY_DIR}/include")

# Options
option(BUILD_TESTS "Build BMUnit tests" OFF)

# Print build configuration
tinyos_print_config()

# Copy compile_commands.json to project root for clangd
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    add_custom_target(
        copy_compile_commands ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
        BYPRODUCTS ${CMAKE_SOURCE_DIR}/compile_commands.json
        COMMENT "Copying compile_commands.json to project root"
    )
endif()

# Testing framework
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(bmunit)
endif()

# Library modules
add_subdirectory(lib)

# Architecture layer
add_subdirectory(arch)

# Boot module
add_subdirectory(boot)

# Core kernel modules
add_subdirectory(kernel)

# Memory management
add_subdirectory(mm)

# Device drivers (future)
# add_subdirectory(drivers)

# Print build configuration
message(STATUS "========================================")
message(STATUS "TinyOS Build Configuration")
message(STATUS "========================================")
message(STATUS "C Standard: C${CMAKE_C_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Assembler: ${CMAKE_ASM_NASM_COMPILER}")
message(STATUS "========================================")
