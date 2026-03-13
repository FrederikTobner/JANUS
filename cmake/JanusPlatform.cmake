# JANUS Platform Detection and Configuration
# Validates toolchain, sets arch-specific flags and common compiler flags
#
# Expects: JANUS_TARGET_ARCH set (by toolchain file, preset, or -D flag)
# Expects: CMAKE_C_COMPILER set (by toolchain file or CMake auto-detection)

include_guard(GLOBAL)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(JANUS_HOST_LINUX TRUE)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(JANUS_HOST_WINDOWS TRUE)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(JANUS_HOST_MACOS TRUE)
else()
    message(FATAL_ERROR "Unable to detect host system: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

set(JANUS_TARGET_PLATFORM "elf")

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(JANUS_COMPILER_CLANG TRUE)
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(JANUS_COMPILER_GCC TRUE)
else()
    message(FATAL_ERROR
        "Unsupported compiler: ${CMAKE_C_COMPILER_ID}.\n"
        "JANUS requires Clang or GCC.")
endif()

# Architecture-specific configuration
include(${CMAKE_SOURCE_DIR}/cmake/arch/${JANUS_TARGET_ARCH}/JanusPlatform.cmake)

# Build type detection
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug, Release, MinSizeRel)" FORCE)
endif()

# Export compile commands for clangd/IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generate compile_commands.json" FORCE)

# Generate build timestamp
string(TIMESTAMP JANUS_BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP JANUS_BUILD_TIME "%H:%M:%S")

# Common compiler flags for all kernel code
set(JANUS_COMPILE_OPTIONS_COMMON
    ${JANUS_ARCH_FLAGS}
    -nostdlib
    -ffreestanding
    -fno-builtin
    -fno-stack-protector
    -Wall
    -Wextra
    -Werror
    # Additional warnings for stricter code quality
    -Wconversion
    -Wimplicit
    -Wcast-qual
    -Wpointer-arith
)

# Debug-specific flags
set(JANUS_COMPILE_OPTIONS_DEBUG
    -g3
    -gdwarf-4
    -O0
    -DDEBUG
)

# Release-specific flags
set(JANUS_COMPILE_OPTIONS_RELEASE
    -O2
    -DNDEBUG
)

# MinSizeRel flags (optimize for size)
set(JANUS_COMPILE_OPTIONS_MINSIZEREL
    -Os
    -DNDEBUG
)

# Export configuration
set(JANUS_PLATFORM_LOADED TRUE)

