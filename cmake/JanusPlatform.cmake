# JANUS Platform Detection and Configuration
# Defines platform-specific settings and common compiler flags
#
# This file is included TWICE:
# 1. Before project() - to set up cross-compiler (Phase 1)
# 2. After project() - to configure compiler flags (Phase 2)
#
# Use JANUS_PLATFORM_PHASE to track which phase we're in.


# Detect host platform
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(JANUS_HOST_LINUX TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(JANUS_HOST_WINDOWS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(JANUS_HOST_MACOS TRUE)
else()
    # Pre-project, system name might not be set
    set(JANUS_HOST_LINUX TRUE)  # Assume Linux for now
endif()

# Target architecture (x86_64 or aarch64)
set(JANUS_TARGET_PLATFORM "elf")

# Validate target architecture
if(NOT JANUS_TARGET_ARCH STREQUAL "x86_64" AND NOT JANUS_TARGET_ARCH STREQUAL "aarch64")
    message(FATAL_ERROR "Invalid JANUS_TARGET_ARCH: ${JANUS_TARGET_ARCH}. Must be 'x86_64' or 'aarch64'.")
endif()

# Cross-compilation setup for aarch64
if(JANUS_TARGET_ARCH STREQUAL "aarch64")
    # Try to find Clang first
    find_program(AARCH64_CLANG clang)
    find_program(AARCH64_LD aarch64-linux-gnu-ld)
    find_program(AARCH64_OBJCOPY aarch64-linux-gnu-objcopy)
    # Try to find GCC as fallback
    find_program(AARCH64_GCC aarch64-linux-gnu-gcc)
    if(AARCH64_CLANG)
        set(CMAKE_C_COMPILER ${AARCH64_CLANG})
        set(CMAKE_ASM_COMPILER ${AARCH64_CLANG})
        set(JANUS_AARCH64_CLANG TRUE)
        message(STATUS "Cross-compiling for aarch64 using Clang: ${AARCH64_CLANG}")
    elseif(AARCH64_GCC)
        set(CMAKE_C_COMPILER ${AARCH64_GCC})
        set(CMAKE_ASM_COMPILER ${AARCH64_GCC})
        set(JANUS_AARCH64_GCC TRUE)
        message(STATUS "Cross-compiling for aarch64 using GCC: ${AARCH64_GCC}")
    else()
        message(FATAL_ERROR 
            "No suitable aarch64 cross-compiler found. Install clang or gcc cross-compiler:\n"
            "  Arch:          sudo pacman -S aarch64-linux-gnu-gcc clang\n"
            "  Debian/Ubuntu: sudo apt install gcc-aarch64-linux-gnu clang\n"
            "  Fedora:        sudo dnf install gcc-aarch64-linux-gnu clang")
    endif()
    
    set(CMAKE_LINKER "${AARCH64_LD}" CACHE STRING "AArch64 linker" FORCE)
    set(CMAKE_OBJCOPY ${AARCH64_OBJCOPY})
    
    # Multiboot2 is x86_64 only
    set(JANUS_BOOT_PROTOCOLS "limine" CACHE STRING "Boot protocols (aarch64 supports limine only)" FORCE)
else()
    # Boot protocols to include in kernel (both by default for x86_64)
    set(JANUS_BOOT_PROTOCOLS "multiboot2;limine" CACHE STRING "Boot protocols to support (semicolon-separated)")
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(JANUS_COMPILER_CLANG TRUE)
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(JANUS_COMPILER_GCC TRUE)
else()
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_C_COMPILER_ID}. JANUS requires Clang or GCC.")
endif()

# Build type detection
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug, Release, MinSizeRel)" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Target: ${JANUS_TARGET_ARCH}-${JANUS_TARGET_PLATFORM}")
message(STATUS "Boot protocols: ${JANUS_BOOT_PROTOCOLS}")

# Export compile commands for clangd/IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generate compile_commands.json" FORCE)

# Architecture-specific compiler flags
if(JANUS_TARGET_ARCH STREQUAL "x86_64")
    # x86_64 specific flags
    if(JANUS_COMPILER_CLANG)
        set(JANUS_ARCH_FLAGS
            -target x86_64-elf
            -mno-red-zone
        )
    else()
        # GCC x86_64
        set(JANUS_ARCH_FLAGS
            -m64
            -march=x86-64
            -mno-red-zone
        )
    endif()
elseif(JANUS_TARGET_ARCH STREQUAL "aarch64")
    if(JANUS_COMPILER_CLANG)
        set(JANUS_ARCH_FLAGS
            --target=aarch64-elf
            -march=armv8-a
            -mgeneral-regs-only
        )
        # Clang needs explicit linker flags for aarch64 when cross-compiling
        set(CMAKE_EXE_LINKER_FLAGS "--target=aarch64-elf -fuse-ld=${AARCH64_LD}" CACHE STRING "AArch64 linker flags for Clang" FORCE)
    else()
        # GCC aarch64
        set(JANUS_ARCH_FLAGS
            -march=armv8-a
            -mgeneral-regs-only
        )
    endif()
endif()

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

