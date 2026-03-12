find_program(AARCH64_LD aarch64-linux-gnu-ld)
find_program(AARCH64_OBJCOPY aarch64-linux-gnu-objcopy)
if(NOT AARCH64_LD)
    message(FATAL_ERROR
        "No suitable aarch64 linker found. Install gcc cross-compiler:\n"
            "  Arch:          sudo pacman -S aarch64-linux-gnu-gcc\n"
            "  Debian/Ubuntu: sudo apt install gcc-aarch64-linux-gnu\n"
            "  Fedora:        sudo dnf install gcc-aarch64-linux-gnu")
endif()

if(NOT AARCH64_OBJCOPY)
    message(FATAL_ERROR
        "No suitable aarch64 objcopy found. Install gcc cross-compiler:\n"
            "  Arch:          sudo pacman -S aarch64-linux-gnu-gcc\n"
            "  Debian/Ubuntu: sudo apt install gcc-aarch64-linux-gnu\n"
            "  Fedora:        sudo dnf install gcc-aarch64-linux-gnu")
endif()

if(JANUS_COMPILER_GCC)
    find_program(AARCH64_GCC aarch64-linux-gnu-gcc)
    message(STATUS "Using GCC")
    if(NOT AARCH64_GCC)
        message(FATAL_ERROR
            "No suitable aarch64 gcc cross-compiler found. Install gcc cross-compiler:\n"
            "  Arch:          sudo pacman -S aarch64-linux-gnu-gcc\n"
            "  Debian/Ubuntu: sudo apt install gcc-aarch64-linux-gnu\n"
            "  Fedora:        sudo dnf install gcc-aarch64-linux-gnu")
    else()
        set(CMAKE_C_COMPILER ${AARCH64_GCC})
        set(CMAKE_ASM_COMPILER ${AARCH64_GCC})
    endif()
else()
        message(STATUS "Using clang")
    set(CMAKE_C_COMPILER ${AARCH64_CLANG})
    set(CMAKE_ASM_COMPILER ${AARCH64_CLANG})
endif()
set(CMAKE_LINKER "${AARCH64_LD}" CACHE STRING "AArch64 linker" FORCE)
set(CMAKE_OBJCOPY ${AARCH64_OBJCOPY})

# Multiboot2 is x86_64 only
set(JANUS_BOOT_PROTOCOLS "limine" CACHE STRING "Boot protocols (aarch64 supports limine only)" FORCE)
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
