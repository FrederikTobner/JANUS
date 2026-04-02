# JANUS Platform Detection
# Detects host OS, compiler, validates toolchain
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

# Build type detection
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug, Release, MinSizeRel)" FORCE)
endif()

# Export compile commands for clangd/IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generate compile_commands.json" FORCE)

# Generate build timestamp
string(TIMESTAMP JANUS_BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP JANUS_BUILD_TIME "%H:%M:%S")

# Export configuration
set(JANUS_PLATFORM_LOADED TRUE)
