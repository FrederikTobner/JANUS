cmake_minimum_required(VERSION 3.20)
project(TinyOS C ASM_NASM)

# Set C standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)  # Allow GNU extensions

# Export compile commands for clangd/IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Build all library modules first
add_subdirectory(lib/libkstd)
add_subdirectory(lib/libkbuffer)
add_subdirectory(lib/libkio)

# Architecture layer
add_subdirectory(arch)

# Boot module
add_subdirectory(boot)

# Core kernel modules
add_subdirectory(kernel)

# Memory management (future)
# add_subdirectory(memoryman)

# Device drivers (future)
# add_subdirectory(drivers)

# Testing framework
add_subdirectory(tests)

# Print build configuration
message(STATUS "========================================")
message(STATUS "TinyOS Build Configuration")
message(STATUS "========================================")
message(STATUS "C Standard: C${CMAKE_C_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Assembler: ${CMAKE_ASM_NASM_COMPILER}")
message(STATUS "========================================")
