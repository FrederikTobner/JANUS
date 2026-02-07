# JANUS Platform Detection and Configuration
# Defines platform-specific settings and common compiler flags

# Detect host platform
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(JANUS_HOST_LINUX TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(JANUS_HOST_WINDOWS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(JANUS_HOST_MACOS TRUE)
else()
    message(FATAL_ERROR "Unable to detect underlying system")
endif()

# Target architecture (x86_64 or aarch64)
set(JANUS_TARGET_PLATFORM "elf")


if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(JANUS_COMPILER_CLANG TRUE)
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(JANUS_COMPILER_GCC TRUE)
else()
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_C_COMPILER_ID}. JANUS requires Clang or GCC.")
endif()


# Cross-compilation setup for aarch64
if(JANUS_TARGET_ARCH STREQUAL "aarch64")
   Include(${CMAKE_SOURCE_DIR}/cmake/arch/aarch64/JanusPlatform.cmake) 
elseif(JANUS_TARGET_ARCH STREQUAL "x86_64")
    Include(${CMAKE_SOURCE_DIR}/cmake/arch/x86_64/JanusPlatform.cmake)
    # Boot protocols to include in kernel (both by default for x86_64)
    set(JANUS_BOOT_PROTOCOLS "multiboot2;limine" CACHE STRING "Boot protocols to support (semicolon-separated)")
else()
    message(FATAL_ERROR "Unsupported target architecture: ${JANUS_TARGET_ARCH}")
endif()

# Build type detection
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug, Release, MinSizeRel)" FORCE)
endif()

# Export compile commands for clangd/IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generate compile_commands.json" FORCE)

# Common compiler flags for all kernel code
set(JANUS_COMMON_FLAGS
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
set(JANUS_DEBUG_FLAGS
    -g3
    -gdwarf-4
    -O0
    -DDEBUG

)

# Release-specific flags
set(JANUS_RELEASE_FLAGS
    -O2
    -DNDEBUG
)

# MinSizeRel flags (optimize for size)
set(JANUS_MINSIZEREL_FLAGS
    -Os
    -DNDEBUG
)

# Export configuration
set(JANUS_PLATFORM_LOADED TRUE)

