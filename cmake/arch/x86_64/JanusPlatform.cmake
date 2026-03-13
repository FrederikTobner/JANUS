# x86_64 architecture-specific flags and configuration
# Compiler and linker are set by the toolchain file

# Boot protocols for x86_64 (both Multiboot2 and Limine supported)
set(JANUS_BOOT_PROTOCOLS "multiboot2;limine" CACHE STRING
    "Boot protocols to support (semicolon-separated)")

# Architecture-specific compiler flags
if(JANUS_COMPILER_CLANG)
    set(JANUS_ARCH_FLAGS
        -target x86_64-elf
        -mno-red-zone
    )
elseif(JANUS_COMPILER_GCC)
    # GCC x86_64
    set(JANUS_ARCH_FLAGS
        -m64
        -march=x86-64
        -mno-red-zone
    )
else()
    message(FATAL_ERROR "Unsupported compiler for x86_64: ${CMAKE_C_COMPILER_ID}")
endif()