#[[ 
    CompilerFlags.cmake - JANUS Compiler Flags

    aarch64 architecture-specific flags and configuration
    Compiler and linker are set by the toolchain file
]]

# Boot protocols (aarch64 is UEFI-only, no Multiboot2)
set(JANUS_BOOT_PROTOCOLS "limine" CACHE STRING "Boot protocols (aarch64: limine only)" FORCE)

# Architecture-specific compiler flags
if(JANUS_COMPILER_CLANG)
    set(JANUS_ARCH_FLAGS
        --target=aarch64-elf
        -march=armv8-a
        -mgeneral-regs-only
    )
elseif(JANUS_COMPILER_GCC)
    # GCC aarch64
    set(JANUS_ARCH_FLAGS
        -march=armv8-a
        -mgeneral-regs-only
    )
else()
    message(FATAL_ERROR "Unsupported compiler for aarch64: ${CMAKE_C_COMPILER_ID}")
endif()
