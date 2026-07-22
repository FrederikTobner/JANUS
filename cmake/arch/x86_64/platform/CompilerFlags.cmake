#[[ 
    CompilerFlags.cmake - JANUS Compiler Flags

    x86_64 architecture-specific flags and configuration
    Compiler and linker are set by the toolchain file
]]

# Boot protocols for x86_64 (both Multiboot2 and Limine supported)
set(JANUS_BOOT_PROTOCOLS "multiboot2;limine" CACHE STRING
    "Boot protocols to support (semicolon-separated)")

# Architecture-specific compiler flags
if(JANUS_COMPILER_CLANG)
    set(JANUS_ARCH_FLAGS
        -target x86_64-elf
        -mcmodel=kernel # Required for kernel code that may be >4GB in size
        -mno-red-zone
        -mno-sse
        -mno-sse2
        -mno-mmx
    )
elseif(JANUS_COMPILER_GCC)
    # GCC x86_64
    # -fno-pic/-fno-pie: Arch Linux's system GCC is configured with --enable-default-pie,
    # but -mcmodel=kernel requires non-PIC code.
    set(JANUS_ARCH_FLAGS
        -m64
        -march=x86-64
        -mcmodel=kernel # Required for kernel code that may be >4GB in size
        -fno-pic
        -fno-pie
        -mno-red-zone
        -mno-sse
        -mno-sse2
        -mno-mmx
    )
else()
    message(FATAL_ERROR "Unsupported compiler for x86_64: ${CMAKE_C_COMPILER_ID}")
endif()
