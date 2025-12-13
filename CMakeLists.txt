cmake_minimum_required(VERSION 3.20)
project(TinyOS C ASM_NASM)

# Set C standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)  # Allow GNU extensions

# Export compile commands for clangd/IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Copy compile_commands.json to project root for clangd
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    add_custom_target(
        copy_compile_commands ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
        DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
        COMMENT "Copying compile_commands.json to project root"
    )
endif()

# Include CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Testing support (BMUnit)
option(BUILD_TESTS "Build BMUnit tests" OFF)

if(BUILD_TESTS)
    message(STATUS "BMUnit: Testing enabled")
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
add_subdirectory(memman)

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
