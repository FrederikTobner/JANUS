# TinyOS Platform Detection and Configuration
# Defines platform-specific settings and common compiler flags

include_guard(GLOBAL)

# Detect host platform
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(TINYOS_HOST_LINUX TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(TINYOS_HOST_WINDOWS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(TINYOS_HOST_MACOS TRUE)
else()
    message(WARNING "Unknown host platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Target platform (always x86_64-elf for now)
set(TINYOS_TARGET_ARCH "x86_64" CACHE STRING "Target architecture")
set(TINYOS_TARGET_PLATFORM "elf")

# Boot protocol (wired as a real parameter, but only one supported today)
set(TINYOS_BOOT_PROTOCOL "multiboot2" CACHE STRING "Boot protocol")

# Compiler detection
if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(TINYOS_COMPILER_CLANG TRUE)
    set(TINYOS_TARGET_FLAG "-target")
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(TINYOS_COMPILER_GCC TRUE)
    set(TINYOS_TARGET_FLAG "--target=")
else()
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_C_COMPILER_ID}. TinyOS requires Clang or GCC.")
endif()

# Build type detection
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug, Release, MinSizeRel)" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Target: ${TINYOS_TARGET_ARCH}-${TINYOS_TARGET_PLATFORM}")
message(STATUS "Boot protocol: ${TINYOS_BOOT_PROTOCOL}")

# Export compile commands for clangd/IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generate compile_commands.json" FORCE)

# Common compiler flags for all kernel code
if(TINYOS_COMPILER_CLANG)
    set(TINYOS_COMMON_FLAGS
            -target ${TINYOS_TARGET_ARCH}-${TINYOS_TARGET_PLATFORM}
            -nostdlib
            -ffreestanding
            -fno-builtin
            -fno-stack-protector
            -mno-red-zone
            -Wall
            -Wextra
            -Werror
            # Additional warnings for stricter code quality
            -Wconversion
            -Wimplicit
            -Wcast-qual
            -Wpointer-arith
    )
else()
       # GCC
        if(TINYOS_TARGET_ARCH STREQUAL "x86_64")
             set(GCC_CODE_GEN_FLAG"-m64")
             set(GCC_ARCHITECTURE_FLAG "x86-64")
        else()
            message(FATAL_ERROR "Unsupported target architecture for GCC: ${TINYOS_TARGET_ARCH}")
        endif()
        set(TINYOS_COMMON_FLAGS
            ${GCC_CODE_GEN_FLAG}
            -march=${GCC_ARCHITECTURE_FLAG}
            -nostdlib
            -ffreestanding
            -fno-builtin
            -fno-stack-protector
            -mno-red-zone
            -Wall
            -Wextra
            -Werror
            #Additional warnings for stricter code quality
            -Wconversion
            -Wimplicit
            -Wcast-qual
            -Wpointer-arith
    )
endif()

# Debug-specific flags
set(TINYOS_DEBUG_FLAGS
    -g3
    -gdwarf-4
    -O0
    -DDEBUG

)

# Release-specific flags
set(TINYOS_RELEASE_FLAGS
    -O2
    -DNDEBUG
)

# MinSizeRel flags (optimize for size)
set(TINYOS_MINSIZEREL_FLAGS
    -Os
    -DNDEBUG
)

# Export configuration
set(TINYOS_PLATFORM_LOADED TRUE)

