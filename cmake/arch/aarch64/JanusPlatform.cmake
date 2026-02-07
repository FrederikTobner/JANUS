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
elseif(AARCH64_GCC)
    set(CMAKE_C_COMPILER ${AARCH64_GCC})
    set(CMAKE_ASM_COMPILER ${AARCH64_GCC})
    set(JANUS_AARCH64_GCC TRUE)
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