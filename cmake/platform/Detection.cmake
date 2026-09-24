#[[ 
    Detection.cmake - JANUS Platform Detection
    Detects host OS, compiler, validates toolchain

    Expects: JANUS_TARGET_ARCH set (by toolchain file, preset, or -D flag)
    Expects: CMAKE_C_COMPILER set (by toolchain file or CMake auto-detection)
]]

include_guard(GLOBAL)

set(JANUS_TARGET_PLATFORM "elf")

if(NOT CMAKE_C_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR
        "Unsupported compiler: ${CMAKE_C_COMPILER_ID}.\n"
        "JANUS requires Clang or GCC.")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug, Release, MinSizeRel)" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generate compile_commands.json" FORCE)

string(TIMESTAMP JANUS_BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP JANUS_BUILD_TIME "%H:%M:%S")

# Export configuration
set(JANUS_PLATFORM_LOADED TRUE)
